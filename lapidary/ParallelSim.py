#! /usr/bin/env python3
import json
import os

from argparse import ArgumentParser
from collections import defaultdict
from datetime import datetime
from fcntl import lockf, LOCK_UN, LOCK_EX
from multiprocessing import Process, cpu_count, Lock, Pool
from pathlib import Path, PosixPath
from pprint import pprint
import progressbar
from progressbar import ProgressBar
from subprocess import Popen, PIPE, DEVNULL
from time import time, sleep

import Experiment, Utils
from SpecBench import *

class ParallelSim:

    def _kill_stat_files(self):
        possible   = [x / 'stats.txt' for x in self.simout_res_path.iterdir()]
        stat_files = [x for x in possible if x.exists()]
        res = [x.unlink() for x in stat_files]
        return res

    def __init__(self, args, append_log_file=False):
        if 'smt' in args.config_name:
            output_dir_parent = Path('smt_simulation_results')
        else:
            output_dir_parent = Path('simulation_results')
        if not output_dir_parent.exists():
            output_dir_parent.mkdir()

        self.simout_res_path = output_dir_parent

        self.summary_path = output_dir_parent / '{}_{}_summary.json'.format(
            args.bench, args.config_name)

        self.summary = defaultdict(dict)
        if self.summary_path.exists() and not args.force_rerun:
            print('\tLoading existing results.')
            with self.summary_path.open() as f:
                raw_dict = json.load(f)
                for k, v in raw_dict.items():
                    self.summary[k] = v
        elif self.summary_path.exists() and args.force_rerun:
            print('\tIgnoring old summary file to force rerun.')

        self.summary['mode']  = args.config_name
        self.summary['bench'] = args.bench
        self.summary['successful_checkpoints'] = 0
        self.summary['failed_checkpoints']     = 0

        assert args.checkpoint_dir is not None

        chkdir = Path(args.checkpoint_dir)
        dirents = Utils.get_directory_entries_by_time(chkdir)
        if 'checkpoints' in self.summary:
            self.chkpts = [x for x in dirents if x.is_dir() and (x / 'system.physmem.store0.pmem').exists()]
            rm_count = 0
            for chk, status in self.summary['checkpoints'].items():
                chk_path = PosixPath(chk)
                if chk_path in self.chkpts and status != 'not run':
                    rm_count += 1
                    print('\tRemoving ' + str(chk_path))
                    print(status)
                    self.chkpts.remove(chk_path)
                    if status == 'failed':
                        self.summary['failed_checkpoints'] += 1
                    elif status == 'successful':
                        self.summary['successful_checkpoints'] += 1
                elif chk_path not in self.chkpts and status != 'not run':
                    if status == 'failed':
                        self.summary['failed_checkpoints'] += 1
                    elif status == 'successful':
                        self.summary['successful_checkpoints'] += 1

            print('\tRemoved {} checkpoints from consideration.'.format(rm_count))
        else:
            self.chkpts = [x for x in dirents if x.is_dir() and (x / 'system.physmem.store0.pmem').exists()]

        exp_args = {}


        invalid_counter = 0
        # Always update this, for it could change!
        self.summary['total_checkpoints'] = len(self.chkpts)

        self.result_files = {}
        if '-' in str(args.bench):
            benches = str(args.bench).partition('-')
            benches = ['--bench', benches[0], benches[2]]
        else:
            benches = ['--bench', args.bench]
        for chkpt in self.chkpts:
            # pmem_file = chkpt / 'system.physmem.store0.pmem'
            # if not pmem_file.exists():
            #     invalid_counter += 1
            #     self.summary['checkpoints'][str(chkpt)] = 'invalid'
            #     #print('{} -- invalid checkpoint, skipping'.format(str(chkpt)))
            #     continue
            self.summary['checkpoints'][str(chkpt)] = 'not run'
            output_dir = output_dir_parent / '{}_{}_{}'.format(
                args.bench, args.config_name, str(chkpt.name))
            
            arg_list = [
                './Experiment.py',
                '--suite', args.suite,
                '--warmup-insts', str(args.warmup_insts),
                '--reportable-insts', str(args.reportable_insts),
                '--start-checkpoint', str(chkpt),
                '--output-dir', str(output_dir)] + benches
            dolma_mode = str(0)
            if 'stt' in args.config_name:
                arg_list += ['--stt']
            if 'default' in args.config_name:
                if 'mem_only' in args.config_name:
                    dolma_mode = str(3)
                else:
                    dolma_mode = str(1)
            elif 'conservative' in args.config_name:
                if 'mem_only' in args.config_name:
                    dolma_mode = str(4)
                else:
                    dolma_mode = str(2)

            if 'smt' in args.config_name:
                arg_list.append('--smt')
            
            arg_list += [
                '--mode', dolma_mode,
            ]
                

            exp_args[str(chkpt)] = arg_list

            result_file = output_dir / 'res.json'
            self.result_files[str(chkpt)] = result_file

        if 'invalid_counter' not in self.summary:
            self.summary['invalid_checkpoints'] = invalid_counter

        if invalid_counter == len(self.chkpts):
            raise Exception('No valid checkpoints to simulate with!')
        elif invalid_counter > 0:
            print('Skipping {} invalid checkpoints'.format(invalid_counter))

        self.num_checkpoints = len(exp_args) + self.summary['successful_checkpoints']
        if args.num_checkpoints is not None:
            self.num_checkpoints = min(args.num_checkpoints, self.num_checkpoints)
            if self.num_checkpoints < args.num_checkpoints:
                print('Warning: Requested {} checkpoints, but only {} are available.'.format(
                    args.num_checkpoints, self.num_checkpoints))

        self.all_proc_args = exp_args
        self.max_procs     = int(args.pool_size)
        self.log_file      = args.log_file
        self.append        = append_log_file
        self.timeout_seconds = (60.0 * 60.0)


    def __del__(self):
        ''' On destruction, output summary to summary file. '''
        with self.summary_path.open('w') as f:
            json.dump(self.summary, f, indent=4)

    @staticmethod
    def _run_process(experiment_args, log_file):
        experiment = Popen(experiment_args, stdin=DEVNULL, stdout=PIPE, stderr=PIPE)
        stdout, stderr = experiment.communicate()

        with open(log_file, 'a') as f:
            try:
                lockf(f, LOCK_EX)
                f.write('-'*80 + os.linesep)
                f.write(' '.join(experiment_args) + os.linesep)
                f.write('STDOUT ' + '-'*40 + os.linesep)
                f.write(stdout.decode('ascii'))
                f.write('STDERR ' + '-'*40 + os.linesep)
                f.write(stderr.decode('ascii'))
            finally:
                lockf(f, LOCK_UN)

    def start(self):
        with open(self.log_file, 'w' if not self.append else 'a') as f:
            f.write('*' * 80 + '\n')
            f.write('Starting simulation run at {}...\n'.format(datetime.utcnow()))

        widgets = [
                    progressbar.Percentage(),
                    ' (', progressbar.Counter(), ' of {})'.format(self.num_checkpoints),
                    ' ', progressbar.Bar(left='[', right=']'),
                    ' ', progressbar.Timer(),
                    ' ', progressbar.ETA(),
                  ]
        with Pool(self.max_procs) as pool, \
             ProgressBar(widgets=widgets, max_value=self.num_checkpoints) as bar:

            bar.start()

            wait_time = 0.001
            failed_counter        = 0
            successful_counter    = self.summary['successful_checkpoints']
            failed_counter        = self.summary['failed_checkpoints']
            remaining_checkpoints = len(self.all_proc_args)
            tasks = {}
            task_results = {}
            task_checkpoint = {}

            def do_visual_update(self):
                counter = min(successful_counter, self.num_checkpoints)
                bar.update(counter)

            self.__class__.do_visual_update = do_visual_update

            while successful_counter < self.num_checkpoints and \
                  remaining_checkpoints > 0:

                needed_checkpoints = max(0, self.num_checkpoints - successful_counter)
                proc_args = Utils.select_evenly_spaced(self.all_proc_args, needed_checkpoints)

                for chkpt in proc_args:
                    assert chkpt in self.all_proc_args
                    del self.all_proc_args[chkpt]

                remaining_checkpoints = len(self.all_proc_args)

                for chkpt, experiment_args in proc_args.items():
                    fn_args = (experiment_args, self.log_file)
                    task = pool.apply_async(ParallelSim._run_process, fn_args)

                    tasks[task] = time()
                    task_results[task] = self.result_files[chkpt]
                    task_checkpoint[task] = chkpt

                finished_tasks = []
                done_waiting = False
                #print('\nStart loop')
                while not done_waiting:
                    done_waiting = True
                    successful   = False
                    max_time     = 0.0
                    for task, start_time in tasks.items():
                        task.wait(wait_time)
                        if task.ready():
                            successful      = True
                            finished_tasks += [task]
                            result_file     = task_results.pop(task)
                            checkpoint      = task_checkpoint.pop(task)
                            if not result_file.exists():
                                failed_counter += 1
                                self.summary['checkpoints'][checkpoint] = 'failed'
                                #print('Aww')
                            else:
                                successful_counter += 1
                                self.summary['checkpoints'][checkpoint] = 'successful'
                                #print('DING')
                        elif successful:
                            done_waiting = False
                            tasks[task] = time()
                            #print('RESET')
                        else:
                            done_waiting = False
                            max_time = max(time() - start_time, max_time)

                    tasks = {t:s for t, s in tasks.items() if t not in finished_tasks}
                    if not successful and max_time > self.timeout_seconds:
                        done_waiting = True
                    if len(tasks) == 0:
                        done_waiting = True
                    if successful_counter >= self.num_checkpoints:
                        done_waiting = True

                    #print('\n{0} tasks, max time of {1:.1f} seconds'.format(
                    #len(tasks), max_time))

                    self.do_visual_update()

            # Now we're waiting on straggler processes:
            #print('\rWaiting for stragglers.\n')
            #print('Waiting for stragglers.\n')
            ready_to_terminate = successful_counter >= self.num_checkpoints
            while not ready_to_terminate:
                ready_to_terminate = True
                finished_tasks = []
                for task, start_time in tasks.items():
                    successful = False
                    task.wait(wait_time)
                    if task.ready():
                        successful = True
                        # Only terminate if completely stagnant
                        ready_to_terminate = False
                        finished_tasks += [task]
                        result_file = task_results.pop(task)
                        checkpoint  = task_checkpoint.pop(task)
                        if not result_file.exists():
                            failed_counter += 1
                            self.summary['checkpoints'][checkpoint] = 'failed'
                        else:
                            successful_counter += 1
                            self.summary['checkpoints'][checkpoint] = 'successful'
                    elif time() - start_time < self.timeout_seconds:
                        ready_to_terminate = False
                    else:
                        finished_tasks += [task]
                        result_file = task_results.pop(task)
                        checkpoint  = task_checkpoint.pop(task)
                        failed_counter += 1
                        self.summary['checkpoints'][checkpoint] = 'failed (timeout)'


                tasks = {t:s for t, s in tasks.items() if t not in finished_tasks}
                self.do_visual_update()

            self.summary['successful_checkpoints'] = successful_counter
            self.summary['failed_checkpoints']     = failed_counter


    @staticmethod
    def add_args(parser):
        SpecBench.add_parser_args(parser)
        Experiment.add_experiment_args(parser)

        parser.add_argument('--checkpoint-dir', '-d',
                            help='Locations of all the checkpoints')
        parser.add_argument('--pool-size', '-p', default=cpu_count(),
                            help='Number of threads to use')
        parser.add_argument('--log-file', '-l', default='log.txt',
                            help='Where to log stdout/stderr from experiment runs.')
        parser.add_argument('--config-name', '-c', default=None,
                            help='Name of config')
        parser.add_argument('--num-checkpoints', '-n', default=None, type=int,
            help='Number of checkpoints to simulate. If None, then all.')
        parser.add_argument('--all-configs', action='store_true',
            help='Run parallel sim for all configurations')
        parser.add_argument('--force-rerun', action='store_true',
            help='Ignore previous summary files and rerun from scratch')

def main():
    parser = ArgumentParser(description='Run a pool of experiments on gem5.')
    ParallelSim.add_args(parser)

    args = parser.parse_args()
    benchmarks = SpecBench.get_benchmarks(args)

    if len(benchmarks) == 1:
        benchmark = benchmarks[0]
    elif len(benchmarks) == 2:
        benchmark = benchmarks[0] + "-" + benchmarks[1]
    else:
        raise Exception('ParallelSim only supports 1 or 2 (SMT) benchmarks')

    print('ParallelSim for {}'.format(benchmark))
    args.bench = benchmark
    try:
        sim = ParallelSim(args)
        sim.start()
    except Exception as e:
        print(e)
        return 1

    return 0

if __name__ == '__main__':
    exit(main())
