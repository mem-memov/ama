# @file
# @brief Compile explicit copy enrollment, external subscription and technical-failure cleanup boundaries.
# Contract: ../../../../docs/implementation/task-copy-and-failure.md; generated output is never a maintained input.
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
TEST_DEFINES := -DTEST_STDLIB_KIND=MODULE_ROOT_STANDARD -DTEST_STDLIB=\"$(ROOT)/stdlib/modules\" -DTEST_SDK=\"$(ROOT)/src\"
ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk
