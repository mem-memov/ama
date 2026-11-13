# @file
# @brief Validate the integer host-size ABI fixture and its explicit module/binding records.
# Contract: ../../../../../docs/implementation/native-abi.md
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/parser/frontend.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
SOURCES += source/io/load.c source/module/resolve/logical.c
include $(ROOT)/tests/leaf.mk
