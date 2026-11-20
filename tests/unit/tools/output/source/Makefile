# @file
# @brief Check source diagnostic JSON attribution and refusal before output against independent decoding.
# Contract: ../../../../../docs/implementation/static-editor-bindings.md.
ROOT := $(abspath ../../../../..)
SOURCES := source/text/manager.c source/text/lines.c source/text/view.c source/unicode/decode.c \
    diagnostics/record/validate.c diagnostics/record/context.c \
    tools/output/json/stream.c tools/output/json/text.c tools/output/json/values.c \
    tools/output/source/span.c tools/output/source/diagnostic.c
include $(ROOT)/tests/leaf.mk
