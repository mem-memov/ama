ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/driver)) \
    compiler/frontend.c diagnostics/render/render.c diagnostics/render/location.c diagnostics/render/text.c
include $(ROOT)/tests/leaf.mk
