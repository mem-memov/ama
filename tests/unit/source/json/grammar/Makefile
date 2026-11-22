# @file
# @brief Verify neutral protocol syntax and exact values without compiler or project-executor ownership.
# Contract: ../../../../../docs/implementation/source-operations.md.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c
SOURCES += $(patsubst $(ROOT)/src/%,%,$(wildcard $(ROOT)/src/source/json/*.c))
include ../../../../leaf.mk
