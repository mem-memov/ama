.RECIPEPREFIX := >

ROOT := $(abspath ../../..)
COMMON := ../../common.c ../common.c ../process.c
SOURCES :=
TEST_DEFINES = -DAMA_COMPILER='"$(ROOT)/ama"' -DTEST_CC='"$(C_COMPILER)"'
include ../../leaf.mk

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

run: compiler
