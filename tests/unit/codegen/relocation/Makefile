# @file
# @brief Publish real template relocations and compile/run the resulting projects with unchanged native root identities.
# Contract: ../../../../docs/implementation/static-relocation.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
SOURCES += $(foreach component,preview paths roots discovery references staging report transaction,$(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/tools/relocation/$(component))))
TEST_DEFINES := -DTEST_STDLIB=\"$(ROOT)/stdlib/modules\" -DTEST_STDLIB_KIND=MODULE_ROOT_STANDARD -DTEST_SDK=\"$(ROOT)/src\"
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk
