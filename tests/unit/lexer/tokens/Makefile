# @file
# @brief Run independently built token, raw-literal and structural-region boundary cases.
# Contract: ../../../../docs/implementation/lexical-tokens.md.
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/lexer)) \
    source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c diagnostics/record/validate.c
include ../../../leaf.mk
