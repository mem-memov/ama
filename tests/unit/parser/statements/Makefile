.RECIPEPREFIX := >

ROOT := $(abspath ../../../..)
COMMON := ../../../common.c ../../common.c ../common.c
SOURCES := \
    $(ROOT)/src/parser/assignment.c \
    $(ROOT)/src/parser/call.c \
    $(ROOT)/src/parser/control.c \
    $(ROOT)/src/parser/declarations.c \
    $(ROOT)/src/parser/statement.c \
    $(ROOT)/src/parser/value.c \
    $(ROOT)/src/lexer/lexer.c \
    $(ROOT)/src/symbols/table.c \
    $(ROOT)/src/graph/graph.c \
    $(ROOT)/src/common/path.c \
    $(ROOT)/src/common/diagnostic.c \
    $(ROOT)/src/common/source.c \
    $(ROOT)/src/common/names.c \
    $(ROOT)/src/codegen/emitter.c
include ../../../leaf.mk
