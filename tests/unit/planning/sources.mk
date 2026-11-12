# @file
# @brief Compose the explicit production dependencies for this component test boundary.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/planning)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/lowering))
