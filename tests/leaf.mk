.RECIPEPREFIX := >

C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O0 -g -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
LTO_FLAGS ?=
TEST_HEADERS := $(wildcard $(ROOT)/src/*/*.h)

.PHONY: all build run clean
all: run
>    $(MAKE) clean

build: generated/test

generated/test: Makefile $(ROOT)/tests/leaf.mk main.c $(COMMON) $(SOURCES) $(TEST_HEADERS)
>    @mkdir -p generated
>    $(C_COMPILER) -I$(ROOT)/src $(C_COMPILER_FLAGS) $(LTO_FLAGS) -UNDEBUG $(TEST_DEFINES) -o $@ main.c $(SOURCES)

run: build
>    ./generated/test

clean:
>    $(RM) -r generated
