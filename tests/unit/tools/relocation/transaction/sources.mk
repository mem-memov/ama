# @file
# @brief Compose real source/staging/library dependencies for independently runnable transaction phase tests.
# Contract: docs/implementation/relocation-transaction.md; generated artifacts are not maintained inputs.
include $(ROOT)/tests/unit/planning/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/identity))
SOURCES += compiler/frontend.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(foreach component,preview paths roots discovery references staging transaction,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
