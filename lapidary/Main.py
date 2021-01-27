#! /usr/bin/env python3
from argparse import ArgumentParser
import json
from glob import glob
from pathlib import Path
from pprint import pprint
from subprocess import run, Popen
import os
import shutil

from DolmaConfig import generate_configs
from SpecBench import SpecBench, Spec2017Bench

from Checkpoints import GDBCheckpoint
from GDBProcess import GDBEngine

import Utils
import random

def get_configs(args):
    return generate_configs(args.config_group)

def get_benchmarks(args):
    benchmarks = Spec2017Bench.BENCHMARKS
    if args.benchmarks is not None:
        benchmarks = args.benchmarks['spec2017']
    return sorted(benchmarks, reverse=args.reverse)

def delete_bad_checkpoints(summary_file):
    import shutil
    count = 0
    with summary_file.open() as f:
        summary = json.load(f)
        if 'checkpoints' in summary:
            for checkpoint, status in summary['checkpoints'].items():
                if 'failed' in status and Path(checkpoint).exists():
                    shutil.rmtree(checkpoint)
                    count += 1
        else:
            print('\tNo checkpoints left to delete!')
    print('\tDeleted {} invalid checkpoints'.format(count))

def get_checkpoint_dirs(dir_paths):
    dirs = [ Path(d) for d in dir_paths ]
    for d in dirs:
        assert d.exists()
    return dirs

def run_all(args):

    checkpoint_root_dir = Path(args.checkpoint_dir)
    assert checkpoint_root_dir.exists()

    configs    = get_configs(args)

    needsSMT = False
    if not args.generate_only:
        for config_class in configs:
            if 'smt' in config_class.name:
                needsSMT = True
                break

    if needsSMT:
        simlog_dir = Path('smt_simlogs')
        res_root   = Path('smt_simulation_results')
    else:
        simlog_dir = Path('simlogs')
        res_root   = Path('simulation_results')
    benchmarks = get_benchmarks(args)
    if not simlog_dir.exists():
        simlog_dir.mkdir()

    needsSMT = False
    if not args.generate_only:
        for config_class in configs:
            if 'smt' in config_class.name:
                needsSMT = True
                break

    max_checkpoints = 0
    if args.max_checkpoints is not None and int(args.max_checkpoints) > 0:
        max_checkpoints = int(args.max_checkpoints)

    for benchmark in benchmarks:
        if needsSMT:
            continue
        checkpoint_dir = checkpoint_root_dir / '{}_gdb_checkpoints'.format(benchmark)

        convert_procs = []

        # print("Preparing pmem files for " + benchmark + "...")
        
        # if max_checkpoints != 0 and max_checkpoints < len(os.listdir(checkpoint_dir)):
        #     sorted_dir = Utils.select_evenly_spaced(os.listdir(checkpoint_dir), max_checkpoints)
        # else:
        #     sorted_dir = sorted(os.listdir(checkpoint_dir))
        
        # for d in sorted_dir:
        #     d_path = checkpoint_dir / d
        #     pmem_file = d_path / 'system.physmem.store0.pmem'
        #     if d_path.is_dir() and not pmem_file.exists():
        #         convert_procs.append(GDBEngine._create_convert_process(d_path, needsSMT))

        # for proc in convert_procs:
        #     proc.join()

        # continue

        for config_class in configs:
            config_name = config_class.name
            config_name = config_name.lower()

            sim_results = res_root / '{}_{}_summary.json'.format(benchmark, config_name)

            print('Simulating {} with {} config.'.format(
                benchmark, config_name))
            log_file = simlog_dir / '{}_{}_simlog.txt'.format(benchmark, config_name)
            sim_args = ['./ParallelSim.py', '--bench', benchmark, '-d',
                        str(checkpoint_dir), '--log-file', str(log_file),'--config-name', str(config_name)]

            if args.max_checkpoints is not None:
                sim_args += ['-n', args.max_checkpoints]

            if args.force_rerun:
                sim_args += ['--force-rerun']

            proc = run(sim_args)
            if proc.returncode:
                print('ParallelSim exited with error: {}'.format(proc.returncode))

            if not proc.returncode and sim_results.exists() and args.delete_bad_checkpoints:
                delete_bad_checkpoints(sim_results)

        # for d in checkpoint_dir.iterdir():
        #     if d.is_dir():
        #         pmem_file = d / 'system.physmem.store0.pmem'
        #         if pmem_file.exists():
        #             Popen(['rm', str(pmem_file)])
                    #pmem_file.unlink()

    # always create merged SMT checkpoints on the fly (space)
    if needsSMT:
        
        bench_counts = None
        with open('smt_mixes.json') as f:
            bench_counts = json.load(f)

        assert bench_counts is not None

        print(bench_counts)

        for i, benchmark1 in enumerate(benchmarks):

            if '500' in benchmark1 or '502' in benchmark1:
                continue

            checkpoint_dir1 = checkpoint_root_dir / '{}_gdb_checkpoints'.format(benchmark1)

            if max_checkpoints != 0 and max_checkpoints < len(os.listdir(checkpoint_dir1)):
                checkpoints1 = Utils.select_evenly_spaced(os.listdir(checkpoint_dir1), max_checkpoints)
            else:
                checkpoints1 = sorted(os.listdir(checkpoint_dir1))

            all_dir = checkpoints1
            max_batch_size = 160

            for x in range(0, 64):
                start = x * max_batch_size
                if start >= len(all_dir):
                    break
                end = min(start + max_batch_size, len(all_dir))
                print(start)
                print(end)
                checkpoints1 = all_dir[start:end]
                print("Preparing pmem files for " + benchmark1 + "...")
                convert_procs = []
                for d in checkpoints1:
                    d_path = checkpoint_dir1 / d
                    pmem_file = d_path / 'system.physmem.store0.pmem'
                    if d_path.is_dir() and not pmem_file.exists():
                        convert_procs.append(GDBEngine._create_convert_process(d_path, needsSMT))

                for proc in convert_procs:
                    proc.join()

                for benchmark2 in bench_counts[benchmark1]:
                    if benchmark2 < benchmark1:
                        continue
                    else:

                        benchmark = benchmark1 + '-' + benchmark2

                        checkpoint_dir2 = checkpoint_root_dir / '{}_gdb_checkpoints'.format(benchmark2)
                        
                        if max_checkpoints != 0 and max_checkpoints < len(os.listdir(checkpoint_dir2)):
                            checkpoints2 = Utils.select_evenly_spaced(os.listdir(checkpoint_dir2), max_checkpoints)
                        else:
                            checkpoints2 = sorted(os.listdir(checkpoint_dir2))

                        checkpoints2 = checkpoints2[start:min(end, len(checkpoints2))]

                        print("Preparing pmem files for " + benchmark2 + "...")
                        convert_procs = []
                        for d in checkpoints2:
                            d_path = checkpoint_dir2 / d
                            pmem_file = d_path / 'system.physmem.store0.pmem'
                            if d_path.is_dir() and not pmem_file.exists():
                                convert_procs.append(GDBEngine._create_convert_process(d_path, needsSMT))

                        for proc in convert_procs:
                            proc.join()

                        continue

                        checkpoint_dir = Path('/mnt/dolma') / '{}_gdb_checkpoints'.format(benchmark)
                        if not checkpoint_dir.exists():
                            checkpoint_dir.mkdir()
                        
                        assert checkpoint_dir.is_dir()
                        print('Merging checkpoints for {}...'.format(benchmark))

                        num_cpts = min(len(checkpoints1), len(checkpoints2))
                        if args.max_checkpoints is not None:
                            num_cpts = min(num_cpts, int(args.max_checkpoints))
                        subprocs = []
                        for k in range(0, num_cpts):
                            pmem_file = checkpoint_dir / checkpoints1[k] / 'system.physmem.store0.pmem'
                            if not pmem_file.exists():
                                subprocs.append(Popen(['./CreateCombined.py', '-a=' + str(checkpoint_dir1 / checkpoints1[k]), '-b=' + str(checkpoint_dir2 / checkpoints2[k]), '-o=' + str(checkpoint_dir / checkpoints1[k])]))
                        for proc in subprocs:
                            proc.wait()


                        for config_class in configs:
                            config_name = config_class.name
                            if 'smt' not in config_name:
                                continue
                            config_name = config_name.lower()

                            sim_results = res_root / '{}_{}_summary.json'.format(benchmark, config_name)

                            print('Simulating {} with {} config.'.format(
                                benchmark, config_name))
                            log_file = simlog_dir / '{}_{}_simlog.txt'.format(benchmark, config_name)
                            sim_args = ['./ParallelSim.py', '--bench', benchmark1, benchmark2, '-d',
                                        str(checkpoint_dir), '--log-file', str(log_file),'--config-name', str(config_name)]

                            if args.max_checkpoints is not None:
                                sim_args += ['-n', args.max_checkpoints]

                            if args.force_rerun:
                                sim_args += ['--force-rerun']

                            proc = run(sim_args)
                            if proc.returncode:
                                print('ParallelSim exited with error: {}'.format(proc.returncode))

                            if not proc.returncode and sim_results.exists() and args.delete_bad_checkpoints:
                                delete_bad_checkpoints(sim_results)
                            
                        #to save space
                        Popen(['rm', '-rf', str(checkpoint_dir)])
            # print("cleaning up pmem files...")
            # for d in checkpoint_dir1.iterdir():
            #     if d.is_dir():
            #         pmem_file = d / 'system.physmem.store0.pmem'
            #         if pmem_file.exists():
            #             Popen(['rm', str(pmem_file)])

def main():
    parser = ArgumentParser(description='Automate runs across multiple benchmarks')
    parser.add_argument('--benchmarks', '-b', nargs='*',
                        action=SpecBench.ParseBenchmarkNames,
                        help='Which benchmarks to run. If empty, run all.')
    parser.add_argument('--config-group', '-g', nargs='?',
                        help='What config group to run')
    parser.add_argument('--config-name', '-c', nargs='?',
                        help='What single config to run')
    parser.add_argument('--force-recreate', action='store_true',
                        help='Force the recreation of checkpoints')
    parser.add_argument('--force-rerun', action='store_true',
                        help='Resimulate benchmarks which have already run.')
    parser.add_argument('--max-checkpoints', '-m', nargs='?',
                        help='Specify the max number of checkpoints to simulate')
    parser.add_argument('--checkpoint-dir', '-d',
                        help='Checkpoint root directories')
    parser.add_argument('--reverse', action='store_true',
                        help='Reverse benchmark order')
    parser.add_argument('--generate-only', action='store_true',
                        help='Only generate benchmarks, no simulations')
    parser.add_argument('--add-stt', action='store_true',
                        help='Also run STT configs.')
    parser.add_argument('--add-inorder', action='store_true',
                        help='Also run in-order.')
    parser.add_argument('--delete-bad-checkpoints', action='store_true',
                        help='If a checkpoint is flakey, delete it!')

    args = parser.parse_args()

    run_all(args)

if __name__ == '__main__':
    exit(main())
