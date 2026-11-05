.RECIPEPREFIX := >

# Makefile for the Ama compiler
#
# Run `make help` for a summary of available targets.

# ---------------------------------------------------------------------------
# Toolchain
# ---------------------------------------------------------------------------

C_COMPILER       ?= gcc
C_COMPILER_FLAGS ?= -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
LINKER_FLAGS     ?=

# ---------------------------------------------------------------------------
# Project layout
# ---------------------------------------------------------------------------

PROJECT_TARGET := ama

PROJECT_SOURCES := $(sort $(wildcard src/*.c src/*/*.c))
PROJECT_OBJECTS := $(patsubst src/%.c,build/%.o,$(PROJECT_SOURCES))
PROJECT_DEPS := $(PROJECT_OBJECTS:.o=.d)
CPPFLAGS += -Isrc
LTO_FLAGS ?= -flto

# ---------------------------------------------------------------------------
# Behaviour
# ---------------------------------------------------------------------------

.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

.PHONY: all help test test-clean clean rebuild

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------

help:
>    @echo "Ama compiler — build targets"
>    @echo ""
>    @echo "  make                Build the compiler (./ama)."
>    @echo "  make all            Same as 'make'."
>    @echo "  make test           Run C unit suites and integration scenarios."
>    @echo "  make clean          Remove build/ and ./ama."
>    @echo "  make rebuild        'clean', then 'all'."
>    @echo "  make help           Show this message."
>    @echo ""
>    @echo "Toolchain overrides (pass on the command line):"
>    @echo "  C_COMPILER=<cmd>            C compiler (default: gcc)."
>    @echo "  C_COMPILER_FLAGS=\"<flags>\"  Compiler flags (default: -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L)."
>    @echo "  LTO_FLAGS=\"<flags>\"  Link-time optimization (default: -flto; empty disables)."
>    @echo "  LINKER_FLAGS=\"<flags>\"      Linker flags (default: empty)."
>    @echo ""
>    @echo "Examples:"
>    @echo "  make"
>    @echo "  make C_COMPILER=clang"
>    @echo "  make C_COMPILER_FLAGS=\"-O0 -g3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L\""
>    @echo "  make clean && make"

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------

all: $(PROJECT_TARGET)

$(PROJECT_TARGET): $(PROJECT_OBJECTS)
>    $(C_COMPILER) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS) -o $@ $^

build/%.o: src/%.c Makefile
>    @mkdir -p $(dir $@)
>    $(C_COMPILER) $(CPPFLAGS) $(C_COMPILER_FLAGS) $(LTO_FLAGS) -MMD -MP -c -o $@ $<

-include $(PROJECT_DEPS)

# ---------------------------------------------------------------------------
# Test
# ---------------------------------------------------------------------------

test: $(PROJECT_TARGET)
>    $(MAKE) -C tests C_COMPILER="$(C_COMPILER)" C_COMPILER_FLAGS="$(C_COMPILER_FLAGS)" LTO_FLAGS="$(LTO_FLAGS)"

test-clean:
>    $(MAKE) -C tests clean

# ---------------------------------------------------------------------------
# Housekeeping
# ---------------------------------------------------------------------------

clean:
>    rm -rf build $(PROJECT_TARGET)

rebuild: clean all
