# @file
# @brief Build strict INI record/shape tests independently of document ownership.
# Contract: ../../../../../docs/implementation/project-configuration.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration/records)) \
    source/configuration/lines/space.c source/path/syntax.c source/path/version.c \
    source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c \
    source/unicode/identifier.c source/unicode/word.c source/unicode/name.c
include ../../../../leaf.mk
