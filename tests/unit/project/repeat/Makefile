.RECIPEPREFIX := >

ROOT := $(abspath ../../../..)
COMMON := ../../../common.c ../../common.c ../common.c
SOURCES := \
    $(ROOT)/src/common/source.c \
    $(ROOT)/src/common/names.c \
    $(ROOT)/src/codegen/emitter.c \
    $(ROOT)/src/codegen/program.c \
    $(ROOT)/src/codegen/native.c \
    $(ROOT)/src/common/diagnostic.c \
    $(ROOT)/src/common/path.c \
    $(ROOT)/src/graph/graph.c \
    $(ROOT)/src/lexer/lexer.c \
    $(ROOT)/src/lexer/sections.c \
    $(ROOT)/src/parser/assignment.c \
    $(ROOT)/src/parser/call.c \
    $(ROOT)/src/parser/control.c \
    $(ROOT)/src/parser/declarations.c \
    $(ROOT)/src/parser/statement.c \
    $(ROOT)/src/parser/value.c \
    $(ROOT)/src/project/compile.c \
    $(ROOT)/src/project/library.c \
    $(ROOT)/src/project/module.c \
    $(ROOT)/src/project/native.c \
    $(ROOT)/src/project/scan.c \
    $(ROOT)/src/symbols/table.c
TEST_DEFINES := -DPROJECT_ROOT='"$(ROOT)"'
include ../../../leaf.mk
