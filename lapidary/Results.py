#! /usr/bin/env python3
from argparse import ArgumentParser
from collections import defaultdict
from IPython import embed
import itertools
import json
from enum import Enum
from math import sqrt
import pandas as pd
from pathlib import Path
from pprint import pprint
import re

import Utils
from SpecBench import *
from Graph import Grapher

import pandas as pd
pd.set_option('display.float_format', lambda x: '%.3f' % x)
#pd.set_option('display.max_rows', None)

class RunType(Enum):
    IN_ORDER      = 'in-order'
    OUT_OF_ORDER  = 'out-of-order'

class Results:
    NORMAL_STATS = ['system.cpu.committedInsts', 'sim_ticks', 'cpi']
    SMT_STATS = ['system.cpu.committedInsts::0', 'system.cpu.committedInsts::1', 'system.cpu.committedInsts::total', 'sim_ticks', 'cpi::0', 'cpi::1', 'cpi::total']

    def __init__(self, runtype, benchmark_name, stats_file):
        assert isinstance(runtype, RunType)
        assert isinstance(benchmark_name, str)
        assert isinstance(stats_file, Utils.StatsFile)
        if runtype == RunType.IN_ORDER:
            self.use_in_order_stats = True
        else:
            self.use_in_order_stats = False
        self.stats_file = stats_file
        self.warmup_stats = None
        self.final_stats = None

    def _get_stats(self):
        raw_stats = pd.Series(self.stats_file.get_current_stats())
        return pd.to_numeric(raw_stats, errors='coerce')

    def get_warmup_stats(self):
        self.warmup_stats = self._get_stats()

    def calcMLP(self):
        accumulator = 0
        numCyclesWithAtLeastSingleOutstandingMemOp = 0

        for k,v in self.final_stats.items():
            if 'outstandingMemOperations::' in k and 'total' not in k:
                bucketIndex = float(k.split('::')[1])
                if bucketIndex == 0:
                    continue
                numItems    = v
                accumulator += bucketIndex * numItems
                numCyclesWithAtLeastSingleOutstandingMemOp += numItems

        if numCyclesWithAtLeastSingleOutstandingMemOp == 0:
            return 1.0 # The minium MLP is always 1.

        return accumulator / numCyclesWithAtLeastSingleOutstandingMemOp

    def get_final_stats(self, smt):
        run_stats = self._get_stats()
        assert len(run_stats)

        self.final_stats = run_stats - self.warmup_stats

        # add IPC
        cycles                  = self.final_stats['sim_ticks'] / 500
        if smt:
            self.final_stats['cpi::0'] = float(cycles) / self.final_stats['system.cpu.committedInsts::0']
            self.final_stats['cpi::1'] = float(cycles) / self.final_stats['system.cpu.committedInsts::1']
            self.final_stats['cpi::total'] = float(cycles) / self.final_stats['system.cpu.committedInsts::total']
            #self.final_stats['ipc'] = float('nan')
            #self.final_stats['cpi'] = float('nan')
        else:
            insts                   = self.final_stats['system.cpu.committedInsts']
            self.final_stats['ipc'] = float(insts) / float(cycles)
            self.final_stats['cpi'] = float(cycles) / float(insts)
            #self.final_stats['cpi::0'] = float('nan')
            #self.final_stats['cpi::1'] = float('nan')
            #self.final_stats['cpi::total'] = float('nan')

        micro_ops               = self.final_stats[ 'sim_ops' ]
        self.final_stats['version'] = 1.0

    def stats(self):
        if len(self.final_stats) == 0:
            raise Exception('Trying to inspect stats before initialization!')
        return self.final_stats

    def human_stats(self, smt):
        if smt:
            stats = Results.SMT_STATS
        else:
            stats = Results.NORMAL_STATS

        output = pd.Series(self.final_stats)
        display = pd.Series()

        for requested_stat in stats:
            display = display.append(output.filter(like=requested_stat))

        return display

    def dump_stats_to_file(self, file_path):
        assert isinstance(file_path, Path)
        with file_path.open('w') as f:
            json_str      = self.stats().to_json()
            json_obj      = json.loads(json_str.encode())
            formatted_str = json.dumps(json_obj, indent=4)
            f.write(unicode(formatted_str))

