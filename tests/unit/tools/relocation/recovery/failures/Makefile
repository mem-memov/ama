# @file
# @brief Exhaust recovery allocation and filesystem failures with independent retry owners.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/tools/relocation/transaction/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/recovery))
include $(ROOT)/tests/leaf.mk
