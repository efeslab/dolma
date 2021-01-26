#!/usr/bin/python3

# Modifies the assembly output of compilation for bpu_reg_dcache_load
# to perform a single unsafe load during mis-speculation, highlighting
# that a single micro-op can transmit a secret

import pathlib
parent_dir = pathlib.Path(__file__).parent.absolute()

# generated with gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
# command: gcc src/bpu_reg_dcache_load_old -S -o bin/bpu_reg_dcache_load_old.s
access_phase = [
    'movl\t(%rax), %eax\n',
    '\ttestl\t%eax, %eax\n',
    '\tje\t.L5\n',
    '\tmovq\t-16(%rbp), %rax\n',
    '\tmovzbl\t(%rax), %edx\n',
    '\tmovzbl\ttemp(%rip), %eax\n',
    '\tandl\t%edx, %eax\n',
    '\tmovb\t%al, temp(%rip)',
]

access_phase_modified = [
    'movl\t(%rax), %eax\n',
    '\tmovq\t-16(%rbp), %rcx\n',
    '\ttestl\t%eax, %eax\n',
    '\tje\t.L5\n',
    '\tmovzbl\t(%rcx), %edx\n',
    '\t;movzbl\ttemp(%rip), %eax\n',
    '\t;andl\t%edx, %eax\n',
    '\t;movb\t%al, temp(%rip)'
]

with open(parent_dir / '../bin/bpu_reg_dcache_load_pre_mod.s', 'r') as asm: 
    data = asm.read().replace(''.join(access_phase), ''.join(access_phase_modified))
        
asm.close()

with open(parent_dir / '../bin/bpu_reg_dcache_load_post_mod.s', 'w') as asm:
    asm.write(data)
asm.close()




    
