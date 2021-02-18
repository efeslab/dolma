# DOLMA
------------------
This is the source code for our USENIX Security paper "DOLMA:
Securing Speculation with the Principle of Transient Non-Observability".
DOLMA is a defense against transient execution attacks, implemented in the [gem5 simulator](https://github.com/gem5/gem5).
When using code from this repository, please be sure to cite [the paper](https://www.usenix.org/conference/usenixsecurity21/presentation/loughlin).

We recommend compiling and running this code on Ubuntu 20.04, as that's where we've tested our
setup. However, you _may_ be able to compile and run on Ubuntu 18.04. See [this document](https://www.gem5.org/documentation/general_docs/building) for the differences in gem5 dependencies on Ubuntu 20.04 versus 18.04.

To install the necessary dependencies on Ubuntu 20.04, run `./install_dependencies_ubuntu20.sh` from the repo's top level directory (requires sudo). For Ubuntu 18.04, run , `./install_dependencies_ubuntu18.sh`.

To compile DOLMA, run `python2 $(which scons) -j$(nproc) build/X86_MESI_Two_Level/gem5.opt`
from the repo's top level directory.

The attacks we used to test DOLMA's security, along with usage instructions,
can be found in `attacks/`. These can be used to verify DOLMA's availability, functionality, and
reproducibility during artifact evaluation. See also `artifact_evaluation/README.md`.
------------------
Changes to gem5
------------------
Most of the changes made for DOLMA can be found in `src/cpu/o3/`. At a high level,
they include adding the infrastructure for tracking speculative status
and enforcing restricted execution for unsafe micro-ops. A good place to
start is `src/cpu/base_dyn_inst.hh`, as this file contains the additional
status flags for instructions added by DOLMA. Note that a few changes are
also made in `src/mem/` in order prevent unsafe memory ops from modifying
the cache hierarchy. All in all, `git grep -il dolma` should reveal
most/all of the files we changed.