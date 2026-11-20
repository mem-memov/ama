# Exercise option children and their composition without linking the frontend.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/driver/options))
SOURCES += driver/request/parse.c source/module/resolve/logical.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c source/memory/arena.c \
    source/text/manager.c source/text/lines.c source/text/view.c
SOURCES += source/path/logical.c source/path/version.c
include $(ROOT)/tests/leaf.mk
