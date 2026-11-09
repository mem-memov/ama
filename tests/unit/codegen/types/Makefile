.RECIPEPREFIX := >

ROOT := $(abspath ../../../..)
COMMON := ../../../common.c ../../common.c ../type_common.c ../../../integration/process.c $(wildcard project/*)
SOURCES := \
    $(ROOT)/src/ast/ast.c \
    $(ROOT)/src/codegen/types.c \
    $(ROOT)/src/codegen/type_plan.c \
    $(ROOT)/src/common/names.c \
    $(ROOT)/src/common/source.c \
    $(ROOT)/src/common/path.c \
    $(ROOT)/src/common/diagnostic.c \
    $(ROOT)/src/semantic/initializers.c \
    $(ROOT)/src/parser/assignment.c \
    $(ROOT)/src/parser/call.c \
    $(ROOT)/src/parser/control.c \
    $(ROOT)/src/parser/statement.c \
    $(ROOT)/src/parser/typed.c \
    $(ROOT)/src/parser/value.c \
    $(ROOT)/src/parser/stream.c \
    $(ROOT)/src/lexer/lexer.c
TEST_DEFINES = -DTEST_CC='"$(C_COMPILER)"' -DTEST_ROOT='"$(ROOT)"'
include ../../../leaf.mk
