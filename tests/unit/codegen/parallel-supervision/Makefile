# @file
# @brief Exercise explicit host supervision over concurrently executing channel actors.
# Contract: ../../../../docs/implementation/parallel-supervision.md
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
TEST_DEFINES += -DTEST_WAIT_NATIVE=\"$(ROOT)/tests/unit/codegen/channel-waits/project/wait.c\" -DTEST_CANCEL_NATIVE=\"$(ROOT)/tests/unit/codegen/channel-waits/project/cancel.c\"
include $(ROOT)/tests/leaf.mk
