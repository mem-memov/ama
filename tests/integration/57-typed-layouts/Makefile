.RECIPEPREFIX := >

ROOT := $(abspath ../../..)
COMMON := ../../common.c ../common.c ../process.c $(filter %.c %.h,$(wildcard project/* project/*/* project/*/*/*))
MODULE_FILES := $(filter-out %.c %.h,$(wildcard project/*/* project/*/*/*))
SOURCES :=
TEST_DEFINES = -DAMA_COMPILER='"$(ROOT)/ama"' -DTEST_CC='"$(C_COMPILER)"' -DTEST_ROOT='"$(ROOT)"'
include ../../leaf.mk

# Extensionless Ama sources must never be rebuilt by implicit C executable rules.
$(MODULE_FILES): ;

.PHONY: compiler
compiler:
>    $(MAKE) -C $(ROOT) all

run: compiler
