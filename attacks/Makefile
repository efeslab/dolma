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
	@cd ../; python2 $(SCONS) -j$(NPROC) build/X86_MESI_Two_Level/gem5.opt
	@mkdir -p $(BIN)
	@mkdir -p $(OUT)

control_mem_dcache_load: $(SRC)/control_mem_dcache_load.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/control_mem_dcache_load

control_reg_dcache_load: $(SRC)/control_reg_dcache_load.c complex_btb requirements
	$(CC) $< -S -o $(BIN)/control_reg_dcache_load_pre_mod.s
	@$(SCRIPTS)/control_reg_dcache_load.py
	$(CC) -static $(BIN)/control_reg_dcache_load_post_mod.s -o $(BIN)/control_reg_dcache_load

control_mem_btb_branch: $(SRC)/control_mem_btb_branch.c basic_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/control_mem_btb_branch

control_mem_icache_branch: $(SRC)/control_mem_icache_branch.c complex_btb requirements
	$(CC) $(CFLAGS) $< -static -o $(BIN)/control_mem_icache_branch

control_mem_dtlb_store: $(SRC)/control_mem_dtlb_store.c complex_btb requirements
	$(CC) $< -S -I ../include/ -o $(BIN)/control_mem_dtlb_store_pre_mod.s
	@$(SCRIPTS)/control_mem_dtlb_store.py
	$(CC) -I ../include/ "../util/m5/m5op_x86.S" -static $(BIN)/control_mem_dtlb_store_post_mod.s -o $(BIN)/control_mem_dtlb_store

data_mem_dcache_load: $(SRC)/data_mem_dcache_load.c complex_btb requirements
	$(CC) $< -S -o $(BIN)/data_mem_dcache_load_pre_mod.s
	@$(SCRIPTS)/data_mem_dcache_load.py
	$(CC) -static $(BIN)/data_mem_dcache_load_post_mod.s -o $(BIN)/data_mem_dcache_load

all: None

None:
	@echo "ERROR: please specify --target from the following list..."
	@echo "\tcontrol_mem_dcache_load"
	@echo "\tcontrol_reg_dcache_load"
	@echo "\tcontrol_mem_icache_branch"
	@echo "\tcontrol_mem_btb_branch"
	@echo "\tcontrol_mem_dtlb_store"
	@echo "\tdata_mem_dcache_load"
	@exit 1

clean:
	@rm -rfv $(BIN)
