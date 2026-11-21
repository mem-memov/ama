# @file
# @brief Compose the actual compiler, source tools and native/library commands for driver boundary tests.
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/driver)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library)) \
    diagnostics/render/render.c diagnostics/render/location.c diagnostics/render/text.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/editor))
# The scaffold already owns its JSON writer dependency; link remaining output children once.
SOURCES += $(filter-out tools/output/json/%,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/output)))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation))
