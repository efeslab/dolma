#! /usr/bin/env python3

import gzip, json, os, re, resource, signal, shutil, subprocess, sys
from elftools.elf.elffile import ELFFile

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

class MergeProcess:
    def __init__(self, check_a, check_b, out_dir):
        self.args = ['gdb', '--batch', '-x', __file__]
        os.environ['CHECKPOINT_A_DIR'] = str(check_a)
        os.environ['CHECKPOINT_B_DIR'] = str(check_b)
        os.environ['OUTPUT_DIR'] = str(out_dir)

    def run(self):
        subprocess.run(self.args)

class CheckpointHandle:
    def __init__(self, chk_dir, base_paddr=0):
        mappings_path = chk_dir / 'mappings.json'
        regs_path = chk_dir / 'regs.json'
        coredump_path = chk_dir / 'gdb.core'

        with mappings_path.open('r') as f:
            self.mappings = json.load(f)
            self.mem_size = 2 * 1024 * 1024 * 1024
            del self.mappings['mem_size']

            if base_paddr != 0:
                for vaddr in self.mappings:
                    self.mappings[vaddr]['paddr'] += base_paddr

        with regs_path.open('r') as f:
            self.regs = json.load(f)

        self.coredump_path = coredump_path

    def _create_mappings(self, expand=False):
        mappings = {}
        pgsize = resource.getpagesize()
        for vaddr in self.mappings:
            mapping = self.mappings[vaddr]
            p = mapping['paddr']
            v = mapping['vaddr']
            s = mapping['size']
            o = mapping['offset']
            f = mapping['flags']
            name = mapping['name']
            index = mapping['index']

            assert str(v) == str(vaddr)

            if expand:
                for off in range(0, s, pgsize):
                    paddr = p + off if p != 0 else 0
                    vaddr = v + off
                    offset = o + off
                    mappings[vaddr] = MemoryMapping(
                            index, paddr, vaddr, pgsize, offset, f, name)
            else:
                mappings[vaddr] = MemoryMapping(index, p, v, s, o, f, name)

        return mappings

    def get_core_elf(self):
        return ELFFile(self.coredump_path.open('rb'))

    def get_dict_mappings(self):
        return self.mappings

    def get_file_mappings(self):
        return self._create_mappings(False)

    def get_template_mappings(self):
        return self._create_mappings(True)

    def get_mem_size(self):
        return self.mem_size

    def get_regs(self):
        return self.regs

class CheckpointMerger:
    def __init__(self, chk_a, chk_b, output_dir):
        self.chk_a = chk_a
        self.chk_b = chk_b
        self.pmem_file = output_dir / 'system.physmem.store0.pmem'
        self.mappings_file = output_dir / 'mappings.json'
        self.cpt_file = output_dir / 'm5.cpt'

        if not output_dir.exists():
            output_dir.mkdir()
        assert output_dir.is_dir()

    def _create_pmem_file(self):
        total_mem_size = 4 * 1024 * 1024 * 1024
        pgsize = resource.getpagesize()

        json_mappings = {}
        json_mappings['mem_size'] = total_mem_size
        json_mappings['checkpoint_0'] = self.chk_a.get_dict_mappings()
        json_mappings['checkpoint_1'] = self.chk_b.get_dict_mappings()

        with self.mappings_file.open('w') as f:
            json.dump(json_mappings, f, indent=4)

        chks = [self.chk_a, self.chk_b]
        with self.pmem_file.open('wb') as pmem_raw:
            # Write out whole file as zeros first
            # print("total memory size is {}".format(total_mem_size))
            pmem_raw.truncate(total_mem_size)

            for chk in chks:
                mappings = chk.get_dict_mappings()
                core_elf = chk.get_core_elf()
                # print("handling chk...")

                ''' Copy the code from Ian. Do we have a better way to write this? '''

                for vaddr, mapping_dict in mappings.items():
                    if vaddr == '0' or vaddr == 'mem_size':
                        continue
                    vaddr = int(vaddr)
                    maybe_file = Path(mapping_dict['name'])
                    if maybe_file.exists() and maybe_file.is_file():
                        for s in core_elf.iter_segments():
                            if s['p_type'] != 'PT_LOAD':
                                continue
                            elf_start_vaddr = int(s['p_vaddr'])
                            elf_max_vaddr = elf_start_vaddr + int(s['p_memsz'])
                            if elf_start_vaddr <= vaddr and vaddr < elf_max_vaddr:
                                continue
                            else:
                                with maybe_file.open('rb') as shared_object:
                                    #print("handling shared object at {}: {}".format(vaddr, str(maybe_file)))
                                    offset = int(mapping_dict['offset'])
                                    size   = int(mapping_dict['size'])
                                    paddr  = int(mapping_dict['paddr'])

                                    shared_object.seek(offset, 0)
                                    pmem_raw.seek(paddr, 0)

                                    buf = shared_object.read(size)
                                    pmem_raw.write(buf)

                # Load everything else
                for s in core_elf.iter_segments():
                    if s['p_type'] != 'PT_LOAD':
                        continue
                    assert s['p_filesz'] == s['p_memsz']
                    assert s['p_memsz'] % pgsize == 0
                    if str(s['p_vaddr']) in mappings:
                        mapping = mappings[str(s['p_vaddr'])]
                        paddr = int(mapping['paddr'])
                        pmem_raw.seek(paddr, 0)

                        mem = s.data()
                        assert len(mem) == s['p_memsz']
                        #print('{}: {} -> {}, size {}'.format(os.getpid(), s['p_vaddr'], paddr, len(mem)))
                        pmem_raw.write(mem)

    def _fill_cpt_template(self):
        regs_a = self.chk_a.get_regs()
        regs_b = self.chk_b.get_regs()

        mem_size = 4 * 1024 * 1024 * 1024

        stack_mapping_0 = [ m for v, m in self.chk_a.get_file_mappings().items() if 'stack' in m.name ]
        stack_mapping_1 = [ m for v, m in self.chk_b.get_file_mappings().items() if 'stack' in m.name ]
        assert len(stack_mapping_0) == 1
        assert len(stack_mapping_1) == 1
        stack_mapping_0 = stack_mapping_0[0]
        stack_mapping_1 = stack_mapping_1[0]

        fill_checkpoint_template(
            template_file = 'check.tmpl/m5-smt.cpt',
            output_file = str(self.cpt_file),
            mappings_0 = self.chk_a.get_template_mappings(),
            mappings_1 = self.chk_b.get_template_mappings(),
            misc_reg_string_0 = regs_a['misc_reg'],
            misc_reg_string_1 = regs_b['misc_reg'],
            int_reg_string_0 = regs_a['int_reg'],
            int_reg_string_1 = regs_b['int_reg'],
            pc_string_0 = regs_a['pc'],
            pc_string_1 = regs_b['pc'],
            next_pc_string_0 = regs_a['next_pc'],
            next_pc_string_1 = regs_b['next_pc'],
            float_reg_string_0 = regs_a['float_reg'],
            float_reg_string_1 = regs_b['float_reg'],
            mem_size = mem_size,
            stack_mapping_0 = stack_mapping_0,
            stack_mapping_1 = stack_mapping_1)

    def merge(self):
        self._fill_cpt_template()
        self._create_pmem_file()

################################################################################

def add_arguments(parser):
    parser.add_argument('--coredump-a-dir', '-a',
        help='The coredump of the first benchmark.')
    parser.add_argument('--coredump-b-dir', '-b',
        help='The coredump of the first benchmark.')
    parser.add_argument('--output-dir', '-o',
        help='The output directory')

def gdb_main():
    check_a_dir = str(os.environ['CHECKPOINT_A_DIR'])
    check_b_dir = str(os.environ['CHECKPOINT_B_DIR'])
    output_dir = str(os.environ['OUTPUT_DIR'])

    chk_a = CheckpointHandle(Path(check_a_dir))
    chk_b = CheckpointHandle(Path(check_b_dir), 2 * 1024 * 1024 * 1024)

    merger = CheckpointMerger(chk_a, chk_b, Path(output_dir))
    merger.merge()

def main():
    parser = ArgumentParser('Create raw checkpoints of a process through GDB')
    add_arguments(parser)

    args = parser.parse_args()
    # print('Merging coredumps {} and {}...'.format(args.coredump_a_dir, args.coredump_b_dir))

    if not args.coredump_a_dir or not args.coredump_b_dir:
        print('Must provide two coredumps')
        raise Exception('Missing file')

    if not args.output_dir:
        print('Must provide output directory')
        raise Exception('Missing output')

    gdbproc = MergeProcess(args.coredump_a_dir, args.coredump_b_dir, args.output_dir)
    gdbproc.run()

if __name__ == '__main__':
    try:
        import gdb
        gdb.execute('help', to_string=True)
    except (ImportError, AttributeError) as e:
        main()
    else:
        gdb_main()
