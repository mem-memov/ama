# Makefile for the Ама compiler
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

.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

.PHONY: all help clean rebuild

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------

help:
	@echo "Ама compiler — build targets"
	@echo ""
	@echo "  make                Build the compiler (./ama)."
	@echo "  make all            Same as 'make'."
	@echo "  make clean          Remove build/ and ./ama."
	@echo "  make rebuild        'clean', then 'all'."
	@echo "  make help           Show this message."
	@echo ""
	@echo "Toolchain overrides (pass on the command line):"
	@echo "  C_COMPILER=<cmd>            C compiler (default: gcc)."
	@echo "  C_COMPILER_FLAGS=\"<flags>\"  Compiler flags (default: -O2 -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L)."
	@echo "  LINKER_FLAGS=\"<flags>\"      Linker flags (default: empty)."
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make C_COMPILER=clang"
	@echo "  make C_COMPILER_FLAGS=\"-O0 -g3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L\""
	@echo "  make clean && make"

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

$(PROJECT_OBJECTS): | build

build:
	mkdir -p build

# ---------------------------------------------------------------------------
# Housekeeping
# ---------------------------------------------------------------------------

clean:
	rm -rf build $(PROJECT_TARGET)

rebuild: clean all