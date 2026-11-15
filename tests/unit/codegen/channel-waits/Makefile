# @file
# @brief Compile cooperative channel waits, cancellation and isolated actor failure.
# Contract: ../../../../docs/implementation/channel-waits.md
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
TEST_DEFINES += -DTEST_WAIT_NATIVE=\"$(CURDIR)/project/wait.c\" -DTEST_CANCEL_NATIVE=\"$(CURDIR)/project/cancel.c\"
include $(ROOT)/tests/leaf.mk
