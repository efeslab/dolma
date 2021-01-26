Instructions for running transient execution attacks with DOLMA in gem5.
------------------------------------------------------------------------
Prequisite: run `install_dependencies.sh` in the repo's top level directory.

Usage: ./main.py --target TARGET --mode MODE --stt STT
where...
    - TARGET is one of the following attacks...
        - bpu_mem_btb_branch
        - bpu_mem_dcache_load
        - bpu_mem_dtlb_store
        - bpu_mem_icache_branch
        - bpu_reg_dcache_load
        - mdu_mem_dcache_load
    - MODE is one of the following...
        - 0: baseline (no protection)
        - 1: DOLMA-Default (M+R)
        - 2: DOLMA-Conservative (M+R)
        - 3: DOLMA-Default (M)
        - 4: DOLMA-Conservative (M)
    - STT is one of the following...
        - 0: Disabled (no changes to DOLMA)
        - 1: Enabled (modifies MODE to behave like STT; ignored for baseline)

NOTE: The `main.py` script is written in python3, while our version of gem5 and
its management scripts (e.g., `se_run_experiment.py`) use python2 for legacy
compilation reasons.