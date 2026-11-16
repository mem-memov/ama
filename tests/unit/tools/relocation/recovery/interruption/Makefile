# @file
# @brief Terminate actual writer processes at every registered preparation/publication/cleanup checkpoint.
ROOT := $(abspath ../../../../../..)
include $(ROOT)/tests/unit/tools/relocation/transaction/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/recovery))
include $(ROOT)/tests/leaf.mk
