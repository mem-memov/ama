# @file
# @brief Compose the explicit production dependencies for this component test boundary.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/ast)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/parser)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/lexer)) \
    source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/unicode/decode.c source/unicode/identifier.c source/unicode/name.c diagnostics/record/validate.c
