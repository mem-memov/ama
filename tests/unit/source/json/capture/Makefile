# @file
# @brief Verify coherent capture ownership and exact primitive failures independently of request/response schemas.
# Contract: ../../../../../docs/implementation/source-protocol.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json))
include ../../../../leaf.mk
