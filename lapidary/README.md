This directory contains an example deconstructed version of Lapidary
tailored for use with DOLMA and SPEC CPU 2017.

Lapidary is a simulation sampling framework that converts periodic GDB
coredumps from a benchmark’s execution on real hardware into gem5 checkpoints.

For more information on Lapidary, the latest version, and for use with an arbitrary setup, see
https://github.com/efeslab/lapidary.

The workflow is as follows...

1. Run `install_dependencies.sh` in this repo's TLD
2. Place and build a copy of SPEC CPU 2017 at `../spec-cpu2017`
3. Run `make all` in this directory
4. Run `make install` in this directory
5. Create GDB checkpoints (cpts) for use with lapidary via `GDBProcess.py`
6. Run experiments via `Main.py`
7. Parse the results via `Results.py`

Example `GDBProcess.py` usage
- `./GDBProcess.py --bench BENCH -m NUM_CPTS_TO_GEN -d CPT_DIR --interval SECONDS_BETWEEN_CPTS`

Example `Main.py` usage:
- `./Main.py -g GROUP -d CPT_DIR -m NUM_CPTS_TO_SIM -b BENCH`

Example `Results.py` usage
- `./Results.py process` (generates `report.json`)