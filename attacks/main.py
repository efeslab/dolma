#!/usr/bin/env python3

import os
from os.path import join as joinpath
from argparse import ArgumentParser
from pathlib import Path
import subprocess

from pprint import pprint
import IPython

gem5_dir    = Path('..')
gem5_opt    = gem5_dir / 'build' / 'X86_MESI_Two_Level' / 'gem5.opt'
gem5_script = Path('.') / 'se_run_experiment.py' #This script will call RunExperiment below
   
def run_binary_on_gem5(bin_path, args):
    debugStartCycle   = 0
    
    if args.smt:
        se_py_args = [
                      '--mem-type', 'SimpleMemory',
                      '--smt',
                      '--cmd', str(bin_path) + ";" + str(bin_path),
                      '--cpu-type', 'DerivO3CPU',
                      '--l1d_size', '32kB', 
                      '--l1i_size', '32kB', 
                      '--l2_size',  '256kB', 
                      '--l2cache',
                      '--caches',
                      '--mode', str(args.mode),
                      '--stt', str(args.stt)
                      ]
    else:
        se_py_args = [
                      '--mem-type', 'SimpleMemory',
                      '--cmd', bin_path,
                      '--cpu-type', 'DerivO3CPU',
                      '--l1d_size', '32kB', 
                      '--l1i_size', '32kB', 
                      '--l2_size',  '256kB', 
                      '--l2cache',
                      '--caches',
                      '--mode', str(args.mode),
                      '--stt', str(args.stt)
                      ]

    gem5_args = [str(gem5_opt),
                     '--debug-start=%d' % debugStartCycle,
                     str(gem5_script) ] + se_py_args

    ret = -1
    if 'bpu_mem_dtlb_store' in str(bin_path):
        with open('bpu_mem_dtlb_store.out', 'w') as f:
            ret = subprocess.call(gem5_args, stdout=f)
        if ret == 0:
            parse_tlb_log_args = [
                './scripts/parse_tlb_logs.py',
                '--log',
                './bpu_mem_dtlb_store.out',
            ]
            ret = subprocess.call(parse_tlb_log_args)
        else:
            print("ERROR: TLB attack process exited non-zero")
    else:
        ret = subprocess.call(gem5_args)
    
    return ret

def Make(target):
    ret = os.system("make {}".format(target))
    if ret != 0:
        raise Exception( "make FAILED with code %s" % ret )

def main():
    parser = ArgumentParser(description=
      'Run a standard gem5 configuration with a custom binary.')

    parser.add_argument('--target', 
                        help='Target name (see Makefile for available targets)',
                        default=None, nargs='?')

    parser.add_argument('--mode',
                    help='choose mode for overall testing',
                    default=0, type=int, nargs='?')
    
    parser.add_argument('--smt', 
                        help='Enable smt',
                        default=False, action="store_true")
    
    parser.add_argument('--stt', 
                        help='Enable stt',
                        default=False, action="store_true")

    args = parser.parse_args()

    Make(args.target)
    return run_binary_on_gem5(Path('./bin/{}'.format(args.target)), args)
    

if __name__ == '__main__':
  exit(main())
