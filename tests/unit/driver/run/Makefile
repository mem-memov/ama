ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/driver)) \
    diagnostics/render/render.c diagnostics/render/location.c diagnostics/render/text.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
include $(ROOT)/tests/leaf.mk
