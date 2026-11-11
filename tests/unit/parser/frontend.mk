include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/ast)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/parser)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/lexer)) \
    source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c \
    source/unicode/decode.c source/unicode/identifier.c diagnostics/record/validate.c
