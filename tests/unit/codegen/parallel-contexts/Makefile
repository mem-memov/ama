# @file
# @brief Compile real parallel dispatch, event import and isolated failure under native sanitizers.
# Contract: ../../../../docs/implementation/parallel-context-execution.md
ROOT := $(abspath ../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
TEST_DEFINES := -DTEST_STDLIB=\"$(ROOT)/stdlib/modules\" -DTEST_STDLIB_KIND=MODULE_ROOT_STANDARD -DTEST_SDK=\"$(ROOT)/src\" -DTEST_NATIVE_THREADS
ifneq ($(findstring -fsanitize=thread,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_THREAD_SANITIZER
else ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include $(ROOT)/tests/leaf.mk
