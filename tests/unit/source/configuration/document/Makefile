# @file
# @brief Compose strict INI lines, records, typed tables and arena cleanup.
# Contract: ../../../../../docs/implementation/project-configuration.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration)) \
    source/memory/arena.c source/path/syntax.c source/path/version.c \
    source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c \
    source/unicode/identifier.c source/unicode/word.c source/unicode/name.c
include ../../../../leaf.mk
