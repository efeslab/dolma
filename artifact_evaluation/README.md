# Artifact Evaluation

Per the top level directory's README, the attacks we used to test DOLMA's security can be found in `~/attacks/` (where `~/` is this repo's top level directory). These attacks can be used to verify DOLMA's availability, functionality, and reproducibility during artifact evaluation. See `~/attacks/README.md` for instructions.

DOLMA's gem5 implementation has been tested to run natively on Ubuntu 20.04 atop an Intel x86-64 processor. If desired, DOLMA can can alternatively be run via an Ubuntu 20.04 VM image, pre-installed with DOLMA's dependencies and this repository. The image can be downloaded from https://www.kevinloughlin.org/dolma.tar.gz, which contains booting instructions.
