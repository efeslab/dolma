#!/usr/bin/env python3

import os
from os.path import join as joinpath
from argparse import ArgumentParser
from pathlib import Path
import subprocess

from inspect import currentframe, getframeinfo
from pprint import pprint
import IPython

gem5_dir    = Path('..')
gem5_opt    = gem5_dir / 'build' / 'X86_MESI_Two_Level' / 'gem5.opt'
gem5_script = Path('.') / 'se_run_experiment.py' #This script will call RunExperiment below

def PrintFrameInfo( prefix, frameinfo ):
    print( prefix + "%s:%s:%s" % (      os.path.abspath( frameinfo.filename ), 
                                        frameinfo.function, 
                                        frameinfo.lineno ))

class ExitCause:
    SIMULATION_DONE  = "exiting with last active thread context"
    WORK_BEGIN       = "workbegin"
    WORK_END         = "workend"
    SIMULATION_LIMIT = "simulate() limit reached"

def ToggleFlags(exit_cause, flags):
    import m5 
    for flag in flags:
        if exit_cause == ExitCause.WORK_BEGIN:
            m5.debug.flags[flag].enable() 
        else:
            m5.debug.flags[flag].disable() 

def run_experiment( options, root, system, FutureClass):
    # The following are imported here, since they will be available when RunExperiment 
    # will be called from within gem5:
    from common import Simulation
    import m5
    
    PrintFrameInfo( "Launching ", getframeinfo(currentframe()) )
    system.exit_on_work_items = True
    options.initialize_only = True

    exit_cause = None

    flags = []

    print("**** REAL SIMULATION **** (max ticks: %d)" % options.abs_max_tick )

    Simulation.run(options, root, system, FutureClass)

    while exit_cause != ExitCause.SIMULATION_DONE and \
          exit_cause != ExitCause.SIMULATION_LIMIT:
        exit_event = m5.simulate(options.abs_max_tick)
        exit_cause = exit_event.getCause()

        print( '='*10 + ' Exiting @ tick %i because %s' % ( m5.curTick(), exit_cause ) )
        
        ToggleFlags(exit_cause, flags)

        if exit_cause == ExitCause.WORK_BEGIN:
            print( hex( exit_event.getCode() & 0xFFFFFFFFFFFFFFF ) )
        elif exit_cause == ExitCause.WORK_END:
            print( hex( exit_event.getCode() & 0xFFFFFFFFFFFFFFF ) )

        event_code = exit_event.getCode() & 0xFFFFFFFFFFFFFFF
        print( hex( exit_event.getCode() & 0xFFFFFFFFFFFFFFF ) )
   
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

    return subprocess.call(gem5_args)

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
