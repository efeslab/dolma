
# DOLMA's Gem5-Compatible Transient Execution Attack Suite

Per our [USENIX Security paper](https://www.usenix.org/conference/usenixsecurity21/presentation/loughlin), the goal of this attack suite is to directly demonstrate the ability of DOLMA---as
well as future defenses---to mitigate transient execution attacks across a wide range of covert timing channels (e.g., backend channels such as the D-cache and D-TLB, as well as frontend channels such as the I-cache and BTB), unsafe micro-ops (e.g., memory micro-ops and branches), types of speculation (e.g., control and data), and locations of secrets (e.g., in-register and in-memory).

## Attacks

Upon execution, each program attempts to perform a particular transient execution attack.
The base source program for each attack can be found in `src`, with the file naming
following the convention `specType_secretLocation_channel_uop.c`. In order to produce the precise behavior needed for leakage, select programs are modified at the assembly level by a corresponding `scripts/specType_secretLocation_channel_uop.py` script.

A successful attack is one that creates a clear timing difference based on a secret value. Most programs
output the timer value (e.g., cycles) recorded for each potential guess of the secret value, followed by
whether the attack succeeded or failed.

The exception to this is `control_mem_dtlb_store`, as TLB timing differences (i.e., between a hit and a miss) are not modelled in gem5's system emulation (SE) mode. Instead,
`scripts/parse_tlb_logs.py` scrapes gem5's log of the program's TLB misses. In this case, a successful attack is one that yields a TLB hit based on a secret value, as opposed to a TLB miss for incorrect guesses.

## Instructions

Prequisite: install dependencies (see README.md in the top level directory)

Usage: `./main.py --target TARGET --mode MODE [--stt]`

where...

TARGET is one of the following attacks...

    - control_mem_btb_branch
    - control_mem_dcache_load
    - control_mem_dtlb_store
    - control_mem_icache_branch
    - control_reg_dcache_load
    - data_mem_dcache_load

MODE is one of the following...

    - 0: Baseline (no protection)
    - 1: DOLMA-Default (M+R)
    - 2: DOLMA-Conservative (M+R)
    - 3: DOLMA-Default (M)
    - 4: DOLMA-Conservative (M)

and --stt modifies DOLMA to behave like the state-of-the-art defense [STT](https://github.com/cwfletcher/stt) for comparison; ignored for baseline

NOTE: The `main.py` script is written in python3, while our version of gem5 and
its management scripts (e.g., `se_run_experiment.py`) use python2 for legacy
compilation reasons.

## Expected Results

### control_mem_btb_branch
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `Fail`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `Fail`
- `mode=3,stt` STT-Spectre (M): `Fail`
- `mode=4` DOLMA-Conservative (M): `Fail`
- `mode=4,stt` STT-Futuristic (M): `Fail`

### control_mem_dcache_load
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `Fail`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `Fail`
- `mode=3,stt` STT-Spectre (M): `Fail`
- `mode=4` DOLMA-Conservative (M): `Fail`
- `mode=4,stt` STT-Futuristic (M): `Fail`

### control_mem_dtlb_store
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `Fail`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `Fail`
- `mode=3,stt` STT-Spectre (M): `SUCCESS`
- `mode=4` DOLMA-Conservative (M): `Fail`
- `mode=4,stt` STT-Futuristic (M): `SUCCESS`

### control_mem_icache_branch
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `Fail`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `Fail`
- `mode=3,stt` STT-Spectre (M): `Fail`
- `mode=4` DOLMA-Conservative (M): `Fail`
- `mode=4,stt` STT-Futuristic (M): `Fail`

### control_reg_dcache_load
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `Fail`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `SUCCESS`
- `mode=3,stt` STT-Spectre (M): `SUCCESS`
- `mode=4` DOLMA-Conservative (M): `SUCCESS`
- `mode=4,stt` STT-Futuristic (M): `SUCCESS`

### data_mem_dcache_load
- `mode=0` Baseline: `SUCCESS`
- `mode=1` DOLMA-Default (M+R): `Fail`
- `mode=1,stt` STT-Spectre (M+R): `SUCCESS`
- `mode=2` DOLMA-Conservative (M+R): `Fail`
- `mode=2,stt` STT-Futuristic (M+R): `Fail`
- `mode=3` DOLMA-Default (M): `Fail`
- `mode=3,stt` STT-Spectre (M): `SUCCESS`
- `mode=4` DOLMA-Conservative (M): `Fail`
- `mode=4,stt` STT-Futuristic (M): `Fail`