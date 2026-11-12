# Exercise real publisher phases with test-only, exact POSIX failure positions.
ROOT := $(abspath ../../../../..)
include $(ROOT)/tests/unit/native/scaffold/sources.mk
TEST_DEFINES := -DTEST_SDK=\"$(ROOT)/src\"
override LINKER_FLAGS += -Wl,--wrap=link,--wrap=rename
include $(ROOT)/tests/leaf.mk
