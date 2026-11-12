# @file
# @brief Compose the explicit production dependencies for this component test boundary.
# Contract: ../../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/compiler)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/scaffold)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity)) \
    native/namespace/context.c native/namespace/symbol.c source/mapping/directive.c
