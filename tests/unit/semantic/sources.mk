# @file
# @brief Compose the explicit production dependencies for this component test boundary.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
include $(ROOT)/tests/unit/project/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/semantic))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
# Configure fixture filesystem authority from each actual leaf, not folder depth.
TEST_DEFINES += -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\"
