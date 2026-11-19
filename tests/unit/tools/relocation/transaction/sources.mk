# @file
# @brief Compose real source/staging/library dependencies for independently runnable transaction phase tests.
# Contract: docs/implementation/relocation-transaction.md; generated artifacts are not maintained inputs.
include $(ROOT)/tests/unit/planning/sources.mk
include $(ROOT)/tests/unit/compiler/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(foreach component,preview paths roots discovery references staging report transaction,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
