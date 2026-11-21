# @file
# @brief Build package/local alias isolation and failure contracts.
# Contract: ../../../../../docs/implementation/project-configuration.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/path/normalize)) \
    source/path/logical.c source/path/syntax.c source/path/version.c source/memory/arena.c \
    source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c \
    source/unicode/identifier.c source/unicode/word.c source/unicode/name.c
include ../../../../leaf.mk
