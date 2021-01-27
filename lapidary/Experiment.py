#!/usr/bin/env python3

import os, json, sys
from argparse import ArgumentParser
from pathlib import Path
import subprocess

from inspect import currentframe, getframeinfo
from pprint import pprint
import IPython

import Utils
from Results import *
from SpecBench import *

import pandas as pd
pd.set_option('display.float_format', lambda x: '%.3f' % x)

gem5_dir            = Path('..')
gem5_opt            = gem5_dir / 'build' / 'X86_MESI_Two_Level' / 'gem5.opt'
gem5_script         = Path('.') / 'se_run_experiment.py'

def PrintFrameInfo( prefix, frameinfo ):
    print( prefix + "%s:%s:%s" % (      os.path.abspath( frameinfo.filename ),
                                        frameinfo.function,
                                        frameinfo.lineno ))

class ExitCause:
    SIMULATION_DONE  = "exiting with last active thread context"
    WORK_BEGIN       = "workbegin"
    SIMULATE_LIMIT   = 'simulate() limit reached'
    VALID_STOP       = [SIMULATION_DONE, SIMULATE_LIMIT]

def ToggleFlags( exit_cause, flags ):
    import m5
    if exit_cause == ExitCause.WORK_BEGIN:
        for flagName in flags:
            # print( "Enabling flag %s" % flagName )
            m5.debug.flags[ flagName ].enable()
    else:
        for flagName in flags:
            m5.debug.flags[ flagName ].disable()

def create_gem5_command(args, bin_path, bin_args, cpu_type='DerivO3CPU', extra_se_args=[],
    gem5_debug_args=[], mode=0, outdir_args=[]):
    if not bin_path.exists():
        bins = str(bin_path).split(';')
        if len(bins) != 2:
            print('Error: {} does not exist.'.format(str(bin_path)))
            return -1
        b1 = bins[0]
        b2 = bins[1]
        if not Path(b1).exists() or not Path(b2).exists():
            print('Error: {} does not exist.'.format(str(bin_path)))
            return -1

    se_py_args = [
                  '--mem-type', 'SimpleMemory',
                  '--cmd', str(bin_path),
                  '--cpu-type', str(cpu_type),
                  '--cpu-clock', '2GHz',
                  '--sys-clock', '2GHz',
                  '--l1d_size', '32kB',
                  '--l1d_assoc', '8',
                  '--l1i_size', '32kB',
                  '--l1d_assoc', '8',
                  '--l2_size',  '2MB',
                  '--l2_assoc',  '16',
                  '--l2cache',
                  '--caches',
                  ] + extra_se_args + gem5_debug_args

    if len(bin_args) > 0:
        se_py_args += ['--options', bin_args]

    gem5_opt_args = [ str(gem5_opt) ]
    gem5_args = gem5_opt_args + outdir_args + [str(gem5_script) ] + se_py_args + [
                '--mode', str(mode),]
    if args.smt == True:
        gem5_args.append('--smt')
    if args.stt == True:
        gem5_args.append('--stt')
        gem5_args.append('1')
    return gem5_args


def run_binary_on_gem5(bin_path, bin_args, parsed_args):
    extra_args = [# '--help',
        '--warmup-insts', str(parsed_args.warmup_insts),
        '--reportable-insts', str(parsed_args.reportable_insts),
    ]
    debug_args = []
    outdir_args = []
    if parsed_args.start_checkpoint is not None:
        extra_args += [ '--start-checkpoint', str(parsed_args.start_checkpoint) ]
        mappings_file = Path(parsed_args.start_checkpoint) / 'mappings.json'
        if not mappings_file.exists():
            raise Exception('{} does not exist!'.format(str(mappings_file)))
        mem_size = Utils.get_mem_size_from_mappings_file(mappings_file)
        extra_args += [ '--mem-size', str(mem_size) ]
    else:
        extra_args += [ '--mem-size', str(parsed_args.mem_size) ]

    if parsed_args.output_dir is not None:
        extra_args += [ '--outdir', str(parsed_args.output_dir) ]
        outdir_args += [ '--outdir', str(parsed_args.output_dir) ]

    gem5_args = create_gem5_command(parsed_args, bin_path,
                                         bin_args,
                                         extra_se_args    = extra_args,
                                         gem5_debug_args  = debug_args,
                                         mode             = parsed_args.mode,
                                         outdir_args      = outdir_args)

    print(gem5_args)
    return subprocess.call(gem5_args, env=os.environ)

def do_make(target=''):
    ret = os.system('make {}'.format(target))
    if ret != 0:
        raise Exception('make FAILED with code {}'.format(ret))

def add_experiment_args(parser):
    parser.add_argument('--warmup-insts', type=int,
                        help='Number of instructions to run before reporting stats',
                        default=5000000)
    parser.add_argument('--reportable-insts', type=int,
                        help=('Arguments to supply to simulated executable. If '
                        '-1, run until end of program.'), default=100000)
    parser.add_argument('--output-dir', default=None,
                        help=('Directory for gem5 stats and experiment results. '
                        'Default is gem5/[CHECKPOINT NAME]/{res.json,stats.txt}'))
    parser.add_argument('--in-order', action='store_true',
                        default=False, help='Use timing CPU instead of O3CPU')
    parser.add_argument('--mem-size', '-m', default='1GB',
    help='Size of memory to use. If checkpoint exists, reads from mappings.json')
    
    parser.add_argument('--mode', default=None, help='Internal option of Dolma.')

    parser.add_argument('--smt', 
                        help='Enable smt',
                        default=False, action="store_true")
    parser.add_argument('--stt', 
                        help='Enable STT',
                        default=False, action="store_true")

def main():
    parser = ArgumentParser(description=
      'Run a standard gem5 configuration with a custom binary.')

    SpecBench.add_parser_args(parser)
    add_experiment_args(parser)

    parser.add_argument('--start-checkpoint',
                        default=None, help=('Checkpoint to start simulating from.'
                        'If not given, starts from program beginning'))

    parser.add_argument('--binary', help='compiled binary file path')
    parser.add_argument('--args',
                        help='Arguments to supply to simulated executable',
                        default='', nargs='+')

    args = parser.parse_args()

    if args.bench is not None and args.binary is not None:
        raise Exception('Can only pick one!')

    benchmarks = SpecBench.get_benchmarks(args)
    assert(len(benchmarks) > 0 and len(benchmarks) < 3)

    exp_bin = ""
    exp_args = ""


    for b in benchmarks:
        bench = SpecBench().create(args.suite, b, args.input_type)
        exp_bin += str(bench.binary) + ";"
        exp_args += ' '.join(bench.args) + ";"
    
    exp_bin = exp_bin[:-1]
    exp_args = exp_args[:-1]

    return run_binary_on_gem5(Path(exp_bin), exp_args, args)


if __name__ == '__main__':
    exit(main())
