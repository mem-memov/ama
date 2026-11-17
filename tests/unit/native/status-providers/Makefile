# @file
# @brief Verify exact native error-provider planning independently of project discovery.
# Contract: ../../../../docs/implementation/native-namespaces.md
ROOT := $(abspath ../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(foreach component,source/identity native/status/providers,\
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/$(component)))) \
    source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c source/module/resolve/logical.c
include ../../../leaf.mk
