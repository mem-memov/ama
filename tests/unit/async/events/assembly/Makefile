# @file
# @brief Exercise the native-provider assembly entry rather than separately linked core objects.
# Contract: ../../../../../docs/implementation/native-events.md.
ROOT := $(abspath ../../../../..)
ifneq ($(findstring -DNDEBUG,$(C_COMPILER_FLAGS)),)
TEST_DEFINES += -DTEST_EVENT_RELEASE
endif
LINKER_FLAGS += -pthread -Wl,--wrap=malloc,--wrap=free
include $(ROOT)/tests/leaf.mk
