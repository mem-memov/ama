# @file
# @brief Check exact staged relocation bytes and independently resolved final bindings.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
SOURCES += compiler/frontend.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(foreach component,preview paths roots discovery references staging,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
include $(ROOT)/tests/leaf.mk
