# Makefile for the Ама compiler
#
# Usage:
#   make                   — build ./ama (gcc, -O2)
#   make C_COMPILER=clang  — build with clang
#   make clean             — remove build artifacts
#   make rebuild           — clean, then all
#
# Debug build:
#   make clean
#   make C_COMPILER_FLAGS="-O0 -g3 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L"
#
# Sanitizer build:
#   make clean
#   make C_COMPILER_FLAGS="-O1 -g -fsanitize=address,undefined -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L" \
#        LINKER_FLAGS="-fsanitize=address,undefined"
#
# Requirements: a C11 compiler, POSIX-compliant OS (Linux/macOS).

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

PROJECT_OBJECTS := \
	build/lexer.o \
	build/symbol.o \
	build/emit.o \
	build/parser.o \
	build/graph.o \
	build/backend.o \
	build/driver.o \
	build/main.o

PROJECT_HEADERS := src/ama.h

# ---------------------------------------------------------------------------
# Behaviour
# ---------------------------------------------------------------------------

# Delete partially built .o on compiler failure.
.DELETE_ON_ERROR:

# Default target when invoked as bare `make`.
.DEFAULT_GOAL := all

.PHONY: all clean rebuild

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------

all: $(PROJECT_TARGET)

$(PROJECT_TARGET): $(PROJECT_OBJECTS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) $(LINKER_FLAGS) -o $(PROJECT_TARGET) $(PROJECT_OBJECTS)

build/lexer.o: src/lexer.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/lexer.o src/lexer.c

build/symbol.o: src/symbol.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/symbol.o src/symbol.c

build/emit.o: src/emit.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/emit.o src/emit.c

build/parser.o: src/parser.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/parser.o src/parser.c

build/graph.o: src/graph.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/graph.o src/graph.c

build/backend.o: src/backend.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/backend.o src/backend.c

build/driver.o: src/driver.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/driver.o src/driver.c

build/main.o: src/main.c $(PROJECT_HEADERS)
	$(C_COMPILER) $(C_COMPILER_FLAGS) -c -o build/main.o src/main.c

# Order-only prerequisite: build/ must exist before any .o is produced.
$(PROJECT_OBJECTS): | build

build:
	mkdir -p build

# ---------------------------------------------------------------------------
# Housekeeping
# ---------------------------------------------------------------------------

clean:
	rm -rf build $(PROJECT_TARGET)

rebuild: clean all