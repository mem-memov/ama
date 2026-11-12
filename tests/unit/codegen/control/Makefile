# Exercise scalar control writers and exact IR queries without a frontend/model build.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/codegen/control/emission))
SOURCES += codegen/control/resolution/site.c codegen/control/native.c \
    source/mapping/directive.c source/unicode/decode.c source/memory/arena.c \
    source/text/manager.c source/text/lines.c source/text/view.c
TEST_DEFINES := -DTEST_SDK='"$(ROOT)/src"'
include $(ROOT)/tests/leaf.mk
