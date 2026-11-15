# @file
# @brief Exercise actual native workers, compiled async roots and loader-safe independent code retention.
# Contract: ../../../../docs/implementation/asynchronous-host.md.
ROOT := $(abspath ../../../..)
C_COMPILER ?= gcc
include $(ROOT)/tests/unit/native/scaffold/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/toolchain))
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/library))
TEST_DEFINES := -DTEST_STANDARD_ROOT=\"$(ROOT)/stdlib/modules\" -DTEST_SDK=\"$(ROOT)/src\" -DTEST_ASYNC_HOST=\"$(ROOT)/examples/async-host\" -DTEST_C_COMPILER='"$(C_COMPILER)"'
ifneq ($(findstring -fsanitize=thread,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_THREAD_SANITIZER
else ifneq ($(findstring -fsanitize=,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_SANITIZERS
endif
include ../../../leaf.mk
