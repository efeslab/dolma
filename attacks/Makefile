SRC=./src
SCRIPTS=./scripts
BIN=./bin
OUT=./m5out
INCLUDE += -I ../include/
M5_OPS = "../util/m5/m5op_x86.S"
LIBS=$(M5_OPS)
CC=gcc
CFLAGS=-O0 -g -Werror
CFLAGS+= $(INCLUDE)
CFLAGS+= $(LIBS)
NPROC=$(shell nproc)
SCONS=$(shell which scons)

.PHONY: default all None clean

default: all

complex_btb:
	@sed -i 's/useIndirect = Param.Bool(False, \"Use indirect branch predictor\")/useIndirect = Param.Bool(True, \"Use indirect branch predictor\")/g' ../src/cpu/pred/BranchPredictor.py

basic_btb:
	@sed -i 's/useIndirect = Param.Bool(True, \"Use indirect branch predictor\")/useIndirect = Param.Bool(False, \"Use indirect branch predictor\")/g' ../src/cpu/pred/BranchPredictor.py

requirements:
	@PWD=$(shell pwd)
	@cd ../; env python2 $(SCONS) -j$(NPROC) build/X86_MESI_Two_Level/gem5.opt; cd $(PWD)
	@mkdir -p $(BIN)
	@mkdir -p $(OUT)

bpu_mem_dcache_load: $(SRC)/bpu_mem_dcache_load.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/bpu_mem_dcache_load

bpu_reg_dcache_load: $(SRC)/bpu_reg_dcache_load.c complex_btb requirements
	$(CC) $< -g -S -o $(BIN)/bpu_reg_dcache_load_pre_mod.s
	@$(SCRIPTS)/bpu_reg_dcache_load.py
	$(CC) -static $(BIN)/bpu_reg_dcache_load_post_mod.s -o $(BIN)/bpu_reg_dcache_load

bpu_mem_btb_branch: $(SRC)/bpu_mem_btb_branch.c basic_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/bpu_mem_btb_branch

bpu_mem_icache_branch: $(SRC)/bpu_mem_icache_branch.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/bpu_mem_icache_branch

bpu_mem_dtlb_store: $(SRC)/bpu_mem_dtlb_store.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/bpu_mem_dtlb_store

mdu_mem_dcache_load: $(SRC)/mdu_mem_dcache_load.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/mdu_mem_dcache_load

all: None

None:
	@echo "ERROR: please specify --target from the following list..."
	@echo "\tbpu_mem_dcache_load"
	@echo "\tbpu_reg_dcache_load"
	@echo "\tbpu_mem_icache_branch"
	@echo "\tbpu_mem_btb_branch"
	@echo "\tbpu_mem_dtlb_store"
	@echo "\tmdu_mem_dcache_load"
	@exit 1

clean:
	@rm -rfv $(BIN)
