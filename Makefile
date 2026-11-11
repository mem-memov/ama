.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
LTO_FLAGS ?=
LINKER_FLAGS ?=

include make/sources.mk
PROJECT_SOURCES := $(call source_files,src)
PROJECT_OBJECTS := $(patsubst src/%.c,build/%.o,$(PROJECT_SOURCES))
PROJECT_DEPS := $(PROJECT_OBJECTS:.o=.d)

.PHONY: all test test-clean clean rebuild help
all: ama

ama: $(PROJECT_OBJECTS)
>    $(C_COMPILER) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS) -o $@ $^

build/%.o: src/%.c Makefile make/sources.mk
>    @mkdir -p $(dir $@)
>    $(C_COMPILER) $(CPPFLAGS) -Isrc $(C_COMPILER_FLAGS) $(LTO_FLAGS) -MMD -MP -c $< -o $@

-include $(PROJECT_DEPS)

test: all
>    $(MAKE) -C tests all

test-clean:
>    $(MAKE) -C tests clean

clean: test-clean
>    $(RM) -r build ama

rebuild:
>    $(MAKE) clean
>    $(MAKE) all

help:
>    @echo "make: build the implementation foundation (not a working Ama compiler)"
>    @echo "make test: run independent C/assert unit and integration leaves"
>    @echo "make test-clean: remove test outputs; make clean: remove all build outputs"

export C_COMPILER C_COMPILER_FLAGS LTO_FLAGS
