#!/usr/bin/python3

# Modifies the assembly output of compilation for control_mem_dtlb_store
# to provide necessary pattern for successful TLB/store attack on gem5
# See parse_tlb_logs.py for more details

# generated with gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
# command: gcc src/control_mem_dtlb_store -S -o bin/control_mem_dtlb_store_pre_mod.s
pre_transmit = ['movl	%eax, %eax\n',
        '\tcmpq	%rax, -2104(%rbp)\n',
]

pre_transmit_mod = ['movl	%eax, %eax\n',
        '\tcmpq	%rax, -2104(%rbp)\n',
        '\tmovq	-2104(%rbp), %rax\n',
]

post_transmit = ['leaq	array1(%rip), %rdx\n',
        '\tmovq	-2104(%rbp), %rax\n',
]

post_transmit_mod = ['leaq	array1(%rip), %rdx\n',]

with open('bin/control_mem_dtlb_store_pre_mod.s', 'r') as asm:
    data = asm.read().replace(''.join(pre_transmit), ''.join(pre_transmit_mod)).replace(
        ''.join(post_transmit), ''.join(post_transmit_mod)
    )
asm.close()

with open('bin/control_mem_dtlb_store_post_mod.s', 'w') as asm:
    asm.write(data)
asm.close()




    
