#!/usr/bin/python3

# Modifies the assembly output of compilation for data_mem_dcache_load
# to provide necessary pattern for successful SSB attack on gem5

import pathlib
parent_dir = pathlib.Path(__file__).parent.absolute()

# generated with gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
# command: gcc src/data_mem_dcache_load -S -o bin/data_mem_dcache_load_pre_mod.s
access_phase = [
    'leaq\t0(,%rax,8), %rdx\n',
    '\tleaq\tstr(%rip), %rax\n',
    '\tmovq\t$0, (%rdx,%rax)\n',
    '\tmovq\t24+str(%rip), %rax\n',
    '\tleaq\tprobe(%rip), %rdx\n',
    '\tmovzbl\t(%rax,%rdx), %ebx\n',
    '\tmovzbl\t%bl, %eax\n',
    '\tsall\t$9, %eax\n',
    '\tcltq\n',
    '\tleaq\tcache_test(%rip), %rdx\n',
    '\tmovzbl\t(%rax,%rdx), %eax'
]
        
access_phase_modified = [
    'leaq\t0(,%rax,8), %rdx\n',
    '\tleaq\tprobe(%rip), %rcx\n',
    '\tleaq\tcache_test(%rip), %r10\n',
    '\tleaq\tstr(%rip), %rax\n',
    '\tmovq\t$0, (%rdx,%rax)\n',
    '\tmovq\t24+str(%rip), %rdx\n',
    '\tmovzbl\t(%rdx,%rcx), %ebx\n',
    '\tmovzbl\t%bl, %eax\n',
    '\tsall\t$9, %eax\n',
    '\tmovslq\t%eax, %rdx\n',
    '\tmovzbl\t(%rdx,%r10), %eax'
]


with open(parent_dir / '../bin/data_mem_dcache_load_pre_mod.s', 'r') as asm: 
    data = asm.read().replace(''.join(access_phase), ''.join(access_phase_modified))
        
asm.close()

with open(parent_dir / '../bin/data_mem_dcache_load_post_mod.s', 'w') as asm:
    asm.write(data)
asm.close()