# Build the compiler, recursive C tests and checked Doxygen reference.
# Documentation contracts: docs/implementation/documentation/README.md.
.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
LTO_FLAGS ?=
LINKER_FLAGS ?=
DOXYGEN ?= doxygen
DOCUMENTATION_PYTHON ?= python3

include make/sources.mk
PROJECT_SOURCES := $(call source_files,src)
PROJECT_OBJECTS := $(patsubst src/%.c,build/%.o,$(PROJECT_SOURCES))
PROJECT_DEPS := $(PROJECT_OBJECTS:.o=.d)
CONFIGURATION_DIRECTORY := build
BINARY := ama
OBJECTS := $(PROJECT_OBJECTS)
COMPILE_COMMAND = $(C_COMPILER) $(CPPFLAGS) -Isrc $(C_COMPILER_FLAGS) $(LTO_FLAGS) -MMD -MP
LINK_COMMAND = $(C_COMPILER) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS)
include make/configuration.mk

.PHONY: all test test-clean conformance-check docs docs-check docs-complete docs-reproducible docs-clean clean rebuild help
all: ama

ama: $(PROJECT_OBJECTS) build/link.configuration
>    $(LINK_COMMAND) -o $@ $(PROJECT_OBJECTS)

build/%.o: src/%.c build/compile.configuration Makefile make/sources.mk make/configuration.mk
>    @mkdir -p $(dir $@)
>    $(COMPILE_COMMAND) -c $< -o $@

-include $(PROJECT_DEPS)

test: all
>    $(MAKE) -C tests all

test-clean:
>    $(MAKE) -C tests clean

# Check reviewed current evidence, its exact inputs and actual named C interfaces.
conformance-check:
>    "$(DOCUMENTATION_PYTHON)" tools/conformance/check.py --complete

# Generate the file reference from maintained C comments; no compiler build is needed.
docs: docs/Doxyfile docs-check
>    "$(DOCUMENTATION_PYTHON)" tools/documentation/build.py --doxygen "$(DOXYGEN)"

docs-check:
>    "$(DOCUMENTATION_PYTHON)" tools/documentation/coverage.py

docs-complete:
>    "$(DOCUMENTATION_PYTHON)" tools/documentation/coverage.py --complete

docs-reproducible: docs-check
>    "$(DOCUMENTATION_PYTHON)" tools/documentation/build.py --doxygen "$(DOXYGEN)" --reproducible

docs-clean:
>    $(RM) -r docs/technical build/documentation

clean: test-clean docs-clean
>    $(RM) -r build ama

rebuild:
>    $(MAKE) clean
>    $(MAKE) all

help:
>    @echo "make: build the current Ama compiler (specification conformance is incomplete)"
>    @echo "make test: run independent C/assert unit and integration leaves"
>    @echo "make conformance-check: validate current requirement and acceptance evidence"
>    @echo "make docs: generate the Doxygen C file reference in docs/technical/index.html"
>    @echo "make docs-clean: remove generated documentation and its warning log"
>    @echo "make test-clean: remove test outputs; make clean: remove all build outputs"

export C_COMPILER C_COMPILER_FLAGS CPPFLAGS LTO_FLAGS LINKER_FLAGS
