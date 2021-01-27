#! /usr/bin/env python3

import gzip, json, os, re, resource, signal, shutil, subprocess, sys

try:
    import gdb
    print('Yo!')
    gdb.execute('show version')
except:
    pass

from argparse import ArgumentParser
from elftools.elf.elffile import ELFFile
from multiprocessing import Process
from pathlib import Path
from pprint import pprint
from subprocess import Popen, TimeoutExpired
from IPython import embed

def join(self, timeout):
    try:
        self.wait(timeout)
    except TimeoutExpired:
        pass

def is_alive(self):
    return self.returncode is None

Popen.join     = join
Popen.is_alive = is_alive

from tempfile import NamedTemporaryFile
from time import sleep

sys.path.append('.')
from Checkpoints import GDBCheckpoint
from CheckpointTemplate import *
from SpecBench import *

import CheckpointConvert

GLIBC_PATH = Path('../libc/glibc/build/install/lib').resolve()
LD_LIBRARY_PATH_STR = '{}:/usr/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu'.format(GLIBC_PATH)

class GDBProcess:
    ''' This is to set up a subprocess that runs gdb for us. '''
    def __init__(self, arg_list,
        checkpoint_interval=None,
        checkpoint_instructions=None,
        max_checkpoints=-1,
        root_dir='.',
        compress=False,
        convert=True,
        debug_mode=False):

        set_arg_string = 'set $args = "{}"'.format(' '.join(arg_list))
        print(set_arg_string)
        self.args = ['gdb', '--batch', '-ex', set_arg_string, '-x', __file__]

        if checkpoint_instructions is not None:
            os.environ['CHECKPOINT_INSTS']    = str(checkpoint_instructions)
        elif checkpoint_interval is not None:
            os.environ['CHECKPOINT_INTERVAL'] = str(checkpoint_interval)

        os.environ['CHECKPOINT_MAXIMUM']    = str(max_checkpoints)
        os.environ['CHECKPOINT_DEBUG']      = str(debug_mode)
        os.environ['CHECKPOINT_ROOT_DIR']   = str(root_dir)
        os.environ['CHECKPOINT_COMPRESS']   = str(compress)
        os.environ['CHECKPOINT_CONVERT']    = str(convert)

    def run(self):
        subprocess.run(self.args)


class GDBEngine:
    ''' This class is used by the gdb process running inside gdb.'''

    # start-address end-address size offset name
    VADDR_REGEX_STRING = r'\s*(0x[0-9a-f]+)\s+0x[0-9a-f]+\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s*(.*)'
    VADDR_REGEX = re.compile(VADDR_REGEX_STRING)

    BAD_MEM_REGIONS = ['[vvar]', '[vsyscall]']

    SIGNAL = signal.SIGINT

    def __init__(self,
                 checkpoint_root_dir,
                 compress_core_files,
                 convert_checkpoints):
        import gdb
        self.chk_num = 0
        self.compress_core_files = compress_core_files
        self.compress_processes  = {}
        self.convert_checkpoints = convert_checkpoints
        self.convert_processes   = {}

        #gdb.execute('set env LD_LIBRARY_PATH {}'.format(LD_LIBRARY_PATH_STR))
        # Otherwise long arg strings get mutilated with '...'
        gdb.execute('set print elements 0')
        args = (gdb.execute('print $args', to_string=True)).split(' ')[2:]
        args = [ x.replace('"','').replace(os.linesep, '') for x in args ]
        assert len(args) > 0
        self.binary = args[0]
        self.args = ' '.join(args[1:])
        if checkpoint_root_dir is not None:
            self.chk_out_dir = Path(checkpoint_root_dir) / \
                '{}_gdb_checkpoints'.format(Path(self.binary).name)
        else:
            self.chk_out_dir = Path('{}_gdb_checkpoints'.format(Path(self.binary).name))

        gdb.execute('set print elements 200')


    @classmethod
    def _get_virtual_addresses(cls):
        vaddrs = [0]
        sizes = [resource.getpagesize()]
        offsets = [0]
        names = ['null']
        raw_mappings = gdb.execute('info proc mappings', to_string=True)

        for entry in raw_mappings.split(os.linesep):
            matches = cls.VADDR_REGEX.match(entry.strip())
            if matches:
                vaddrs   += [int(matches.group(1), 16)]
                sizes    += [int(matches.group(2), 16)]
                offsets  += [int(matches.group(3), 16)]
                names    += [str(matches.group(4)).strip()]

        return vaddrs, sizes, offsets, names

    @classmethod
    def _get_memory_regions(cls):
        vaddrs, sizes, offsets, names = cls._get_virtual_addresses()
        paddrs = []
        flags = []
        next_paddr = 0
        for vaddr, size in zip(vaddrs, sizes):
            paddrs += [next_paddr]
            flags += [0]
            next_paddr += size

        return paddrs, vaddrs, sizes, offsets, flags, names

    @classmethod
    def _create_mappings(cls, filter_bad_regions=False, expand=False):
        paddrs, vaddrs, sizes, offsets,flags, names = cls._get_memory_regions()
        assert len(paddrs) == len(vaddrs)
        assert len(paddrs) == len(sizes)
        assert len(paddrs) == len(flags)
        assert len(paddrs) == len(names)
        mappings = {}
        index = 0
        pgsize = resource.getpagesize()
        for p, v, s, o, f, name in zip(paddrs, vaddrs, sizes, offsets, flags, names):
            if filter_bad_regions and name in GDBEngine.BAD_MEM_REGIONS:
                print('Skipping region "{}" (v{}->v{}, p{}->p{})'.format(name,
                  hex(v), hex(v + s), hex(p), hex(p + s)))
                continue
            if expand:
                for off in range(0, s, pgsize):
                    paddr = p + off if p != 0 else 0
                    vaddr = v + off
                    offset = o + off
                    mappings[vaddr] = MemoryMapping(
                        index, paddr, vaddr, pgsize, offset, f, name)
            else:
                mappings[v] = MemoryMapping(index, p, v, s, o, f, name)

            index += 1

        return mappings


    @staticmethod
    def _create_convert_process(checkpoint_dir, is_smt=False):
        gdb_checkpoint = GDBCheckpoint(checkpoint_dir)
        proc = Process(target=CheckpointConvert.convert_checkpoint,
                       args=(gdb_checkpoint, True, is_smt))
        proc.start()
        return proc

    def _dump_core_to_file(self, file_path):
        gdb.execute('set use-coredump-filter off')
        gdb.execute('set dump-excluded-mappings on')
        gdb.execute('gcore {}'.format(str(file_path)))
        if self.compress_core_files:
            print('Creating gzip process for {}'.format(str(file_path)))
            gzip_proc = Popen(['gzip', '-f', str(file_path)])
            self.compress_processes[file_path.parent] = gzip_proc
        elif self.convert_checkpoints:
            print('Creating convert process for {}'.format(str(file_path.parent)))
            convert_proc = GDBEngine._create_convert_process(file_path.parent)
            self.convert_processes[file_path.parent] = convert_proc

    def _dump_regs_to_file(self, regs, file_path):
        json_regs = {}
        json_regs['misc_reg'] = regs.get_misc_reg_string()
        json_regs['int_reg'] = regs.get_int_reg_string()
        json_regs['pc'] = regs.get_pc_string()
        json_regs['next_pc'] = regs.get_next_pc_string()
        json_regs['float_reg'] = regs.get_float_reg_string()

        with file_path.open('w') as f:
            json.dump(json_regs, f, indent=4)

    def _dump_mappings_to_file(self, mappings, mem_size, file_path):
        json_mappings = {'mem_size': mem_size}
        for vaddr, mapping in mappings.items():
            json_mappings[vaddr] = mapping.__dict__

        with file_path.open('w') as f:
            json.dump(json_mappings, f, indent=4)


    def _calculate_memory_size(self, mappings):
        # Avoid OOM errors
        return (4 * 1024 * 1024 * 1024)

    def _create_gem5_checkpoint(self, debug_mode):
        chk_loc = self.chk_out_dir / '{0:0=3d}_check.cpt'.format(self.chk_num)
        if chk_loc.exists():
            print('Warning: {} already exists, overriding.'.format(str(chk_loc)))
        else:
            chk_loc.mkdir(parents=True)
        pmem_name = 'system.physmem.store0.pmem'
        chk_file = 'm5.cpt'

        template_mappings = self._create_mappings(True, expand=True)
        regs = RegisterValues(self.fs_base)

        total_mem_size = self._calculate_memory_size(template_mappings)

        file_mappings = self._create_mappings(True)

        stack_mapping = [ m for v, m in file_mappings.items() if 'stack' in m.name ]
        assert len(stack_mapping) == 1
        stack_mapping = stack_mapping[0]

        fill_checkpoint_template(
            output_file=str(chk_loc / chk_file),
            mappings=template_mappings,
            misc_reg_string=regs.get_misc_reg_string(),
            int_reg_string=regs.get_int_reg_string(),
            pc_string=regs.get_pc_string(),
            next_pc_string=regs.get_next_pc_string(),
            float_reg_string=regs.get_float_reg_string(),
            mem_size=total_mem_size,
            stack_mapping=stack_mapping)

        self._dump_regs_to_file(regs, chk_loc / 'regs.json')
        self._dump_core_to_file(chk_loc / 'gdb.core')
        self._dump_mappings_to_file(file_mappings, total_mem_size,
            chk_loc / 'mappings.json')
        self.chk_num += 1

        if debug_mode:
            import IPython
            IPython.embed()

    @classmethod
    def _interrupt_in(cls, sec):
        def control_c(pid, seconds):
            sleep(seconds)
            os.kill(pid, cls.SIGNAL)

        pid = os.getpid()
        proc = Process(target=control_c, args=(pid, sec))
        proc.start()
        return proc

    def _can_create_valid_checkpoint(self):
        mappings = self._create_mappings()
        regs = RegisterValues(self.fs_base)

        current_pc = int(regs.get_pc_string())
        for vaddr, mapping in mappings.items():
            if current_pc in mapping and mapping.name in GDBEngine.BAD_MEM_REGIONS:
                print('Skipping checkpoint at {} since it is in {} region'.format(
                      hex(current_pc), mapping.name))
                return False

        return True

    @staticmethod
    def _get_fs_base():
        import struct
        lang = gdb.execute('show language', to_string=True).split()[-1].split('"')[0]
        gdb.execute('set language c')
        gdb.execute('compile file -raw get_fs_base.c')
        fs_base = 0
        fs_base_file = Path('fs_base.txt')
        try:
            with fs_base_file.open('rb') as f:
                data    = f.read()[:8]
                print(data)
                print(struct.unpack('Q', data))
                fs_base = struct.unpack('Q', data)[0]
        except:
            pass
        finally:
            if fs_base_file.exists():
                fs_base_file.unlink()
        gdb.execute('set language {}'.format(lang))
        print('Found FS BASE: {} ({})'.format(fs_base, hex(fs_base)))
        return fs_base

    def _run_base(self, debug_mode):
        gdb.execute('set auto-load safe-path /')
        gdb.execute('exec-file {}'.format(self.binary))
        gdb.execute('file {}'.format(self.binary))

        gdb.execute('break main')
        print('Running with args: "{}"'.format(self.args))

        gdb.execute('run {}'.format(self.args))
        self.fs_base = self._get_fs_base()
        if debug_mode:
            import IPython
            IPython.embed()

    def _poll_background_processes(self, wait=False):
        timeout = 0.001
        if wait:
            print('Waiting for background processes to complete before exit.')
            timeout = None
        gzip_complete = []
        for file_path, gzip_proc in self.compress_processes.items():
            gzip_proc.join(timeout)
            if not gzip_proc.is_alive():
                gzip_complete += [file_path]
                if self.convert_checkpoints:
                    print('Creating convert process for {} after gzip'.format(
                        str(file_path)))
                    convert_proc = GDBEngine._create_convert_process(file_path)
                    self.convert_processes[file_path.parent] = convert_proc
        for key in gzip_complete:
            print('Background gzip for {} completed'.format(key))
            self.compress_processes.pop(key)

        convert_complete = []
        for file_path, convert_proc in self.convert_processes.items():
            convert_proc.join(timeout)
            if not convert_proc.is_alive():
                convert_complete += [file_path]
        for key in convert_complete:
            print('Background convert for {} completed'.format(key))
            self.convert_processes.pop(key)

    def _try_create_checkpoint(self, debug_mode):
        self._poll_background_processes()
        if self._can_create_valid_checkpoint():
            print('Creating checkpoint #{}'.format(self.chk_num))
            self._create_gem5_checkpoint(debug_mode)

    def run_time(self, sec_between_chk, max_iter, debug_mode):
        print('Running with {} seconds between checkpoints.'.format(
          sec_between_chk))
        self._run_base(debug_mode)

        while max_iter < 0 or self.chk_num < max_iter:
            try:
                proc = self._interrupt_in(sec_between_chk)
                gdb.execute('continue')
                proc.join()
                self._try_create_checkpoint(debug_mode)
            except (gdb.error, KeyboardInterrupt) as e:
                self._poll_background_processes(True)
                return


    def run_inst(self, insts_between_chk, max_iter, debug_mode):
        print('Running with {} instructions between checkpoints.'.format(
          insts_between_chk))
        self._run_base(debug_mode)

        while max_iter < 0 or self.chk_num < max_iter:
            try:
                gdb.execute('stepi {}'.format(insts_between_chk))
                self._try_create_checkpoint(debug_mode)
            except (gdb.error, KeyboardInterrupt) as e:
                self._poll_background_processes(True)
                return

    def run_load_checkpoint(self):
        gdb.execute("target core {}".format(self.chk_out_dir))

################################################################################

def add_arguments(parser):
    parser.add_argument('--cmd',
        help='Run a custom command instead of a SPEC benchmark', nargs='*')
    parser.add_argument('--compress', default=False, action='store_true',
        help='Compress corefile or not.')
    parser.add_argument('--no-convert', action='store_true',
        help='Do not convert checkpoints in the backgroud')
    parser.add_argument('--max-checkpoints', '-m', default=-1, type=int,
        help='Create a maximum number of checkpoints. -1 for unlimited')
    parser.add_argument('--debug-mode', default=False, action='store_true',
        help='For debugging program execution, run IPython after checkpointing.')
    parser.add_argument('--directory', '-d', default='.',
        help='The parent directory of the output checkpoint directories.')

    group = parser.add_mutually_exclusive_group()
    group.add_argument('--stepi', '-s', nargs='?', type=int,
        help=('Rather than stepping by time, '
          'create checkpoints by number of instructions.'))
    group.add_argument('--interval', type=float, default=1.0,
        help='How often to stop and take a checkpoint.')

def gdb_main():
    checkpoint_root_dir = str(os.environ['CHECKPOINT_ROOT_DIR'])
    max_checkpoints     = int(os.environ['CHECKPOINT_MAXIMUM'])
    debug_mode          = os.environ['CHECKPOINT_DEBUG'] == 'True'
    compress_core_files = os.environ['CHECKPOINT_COMPRESS'] == 'True'
    convert_checkpoints = os.environ['CHECKPOINT_CONVERT'] == 'True'

    engine = GDBEngine(checkpoint_root_dir, compress_core_files,
                       convert_checkpoints)

    if 'CHECKPOINT_INTERVAL' in os.environ:
        checkpoint_interval = float(os.environ['CHECKPOINT_INTERVAL'])
        engine.run_time(checkpoint_interval, max_checkpoints, debug_mode)
    elif 'CHECKPOINT_INSTS' in os.environ:
        checkpoint_instructions = int(os.environ['CHECKPOINT_INSTS'])
        engine.run_inst(checkpoint_instructions, max_checkpoints, debug_mode)
    else:
        print('Exit!')

def main():
    parser = ArgumentParser('Create raw checkpoints of a process through GDB')
    SpecBench.add_parser_args(parser)
    add_arguments(parser)

    args = parser.parse_args()

    SpecBench.maybe_display_spec_info(args)

    if args.cmd and args.bench:
        raise Exception('Can only pick one!')

    arg_list = []
    gdbprocs = []
    if args.cmd:
        assert(False)
        arg_list = args.cmd
        print(arg_list)
        exit(0)
        gdbproc = GDBProcess(arg_list,
                             checkpoint_interval=args.interval,
                             checkpoint_instructions=args.stepi,
                             max_checkpoints=args.max_checkpoints,
                             root_dir=args.directory,
                             compress=args.compress,
                             convert=not args.no_convert,
                             debug_mode=args.debug_mode)
        gdbprocs = [gdbproc]
    else:
        # times currently based on generating 200 checkpoints, unless otherwise noted
        # quote means how many could be generated with 15 second intervals
        cpt_intervals = {
            '500.perlbench_r': 0.1, # GOOD
            '502.gcc_r': 0.35, # GOOD
            '503.bwaves_r': 0.75, # GOOD
            '505.mcf_r': 1.75 / 3, # GOOD 
            '507.cactuBSSN_r': 1.4, # GOOD
            '508.namd_r': 1.25, # GOOD
            '510.parest_r': 2.4, # GOOD
            '511.povray_r': 2.25, # GOOD
            '519.lbm_r': 1.45, # GOOD
            '520.omnetpp_r': 2.15, # GOOD
            '521.wrf_r': 3.1, # GOOD
            '523.xalancbmk_r': 1.0, # GOOD
            '525.x264_r': 1.0, # GOOD
            '526.blender_r': 0.65, # GOOD
            '527.cam4_r': 2.0, # GOOD
            '531.deepsjeng_r': 1.6, # GOOD
            '538.imagick_r': 2.35, # GOOD
            '541.leela_r': 2.5, # GOOD
            '544.nab_r': 1.75, # GOOD
            '548.exchange2_r': 1.5, # GOOD
            '549.fotonik3d_r': 2.15, # GOOD
            '554.roms_r': 1.9, # GOOD
            '557.xz_r': 1.6, # GOOD
        }
        # cpt_intervals = {
        #     '500.perlbench_r': 0.1,
        #     '502.gcc_r': 0.3,
        #     '503.bwaves_r': 0.7,
        #     '505.mcf_r': 1.0, # 24
        #     '507.cactuBSSN_r': 1.0, # 19
        #     '508.namd_r': 1.0, # 17
        #     '510.parest_r': 1.0, # 33
        #     '511.povray_r': 1.0, # 96
        #     '519.lbm_r': 1.0, # 20
        #     '520.omnetpp_r': 1.0, # 29
        #     '521.wrf_r': 1.0, # 42
        #     '523.xalancbmk_r': 1.0,
        #     '525.x264_r': 0.1,
        #     '526.blender_r': 0.65,
        #     '527.cam4_r': 1.0,
        #     '531.deepsjeng_r': 1.0, # 22
        #     '538.imagick_r': 1.0, # 31
        #     '541.leela_r': 1.0, # 48
        #     '544.nab_r': 1.0, # 24
        #     '548.exchange2_r': 1.0, # 95
        #     '549.fotonik3d_r': 1.0, # 29
        #     '554.roms_r': 1.0, # 26
        #     '557.xz_r': 1.0, # 24
        # }
        if args.bench is None:
            benchmarks = ['500.perlbench_r',
                '502.gcc_r',
                '503.bwaves_r',
                '505.mcf_r',
                '507.cactuBSSN_r',
                '508.namd_r',
                '510.parest_r',
                '511.povray_r',
                '519.lbm_r',
                '520.omnetpp_r',
                '521.wrf_r',
                '523.xalancbmk_r',
                '525.x264_r',
                '526.blender_r',
                '527.cam4_r',
                '531.deepsjeng_r',
                '538.imagick_r',
                '541.leela_r',
                '544.nab_r',
                '548.exchange2_r',
                '549.fotonik3d_r',
                '554.roms_r',
                '557.xz_r']
        else:
            benchmarks = SpecBench.get_benchmarks(args)
        for benchmark in benchmarks:
            print('Setting up process for {} (interval = {} seconds)...'.format(benchmark, cpt_intervals[benchmark]))
            bench = SpecBench().create(args.suite, benchmark, args.input_type)
            arg_list = [str(bench.binary)] + bench.args

            gdbproc = GDBProcess(arg_list,
                                 checkpoint_interval=cpt_intervals[benchmark],
                                 checkpoint_instructions=args.stepi,
                                 max_checkpoints=args.max_checkpoints,
                                 root_dir=args.directory,
                                 compress=args.compress,
                                 convert=not args.no_convert,
                                 debug_mode=args.debug_mode)
            gdbprocs += [gdbproc]

    for gdbproc in gdbprocs:
        gdbproc.run()

if __name__ == '__main__':
    try:
        import gdb
        gdb.execute('help', to_string=True)
    except (ImportError, AttributeError) as e:
        main()
    else:
        gdb_main()
