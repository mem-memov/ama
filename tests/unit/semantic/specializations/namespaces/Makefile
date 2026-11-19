# @file
# @brief Compose exact static namespace evidence with ordinary constant operation checks.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler/specializations))
include ../../../../leaf.mk
