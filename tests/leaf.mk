.RECIPEPREFIX := >
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

C_COMPILER ?= gcc
C_COMPILER_FLAGS ?= -O0 -g -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
LTO_FLAGS ?=
LINKER_FLAGS ?=

LOCAL_SOURCES ?= main.c
TEST_OBJECTS := $(patsubst %.c,generated/objects/local/%.o,$(LOCAL_SOURCES)) \
    $(patsubst %.c,generated/objects/production/%.o,$(SOURCES))
TEST_DEPS := $(TEST_OBJECTS:.o=.d)

.PHONY: all build run clean
all: run
>    $(MAKE) clean

build: generated/test

generated/test: $(TEST_OBJECTS)
>    $(C_COMPILER) $(C_COMPILER_FLAGS) $(LTO_FLAGS) $(LINKER_FLAGS) -o $@ $^

generated/objects/local/%.o: %.c Makefile $(ROOT)/tests/leaf.mk
>    @mkdir -p $(dir $@)
>    $(C_COMPILER) $(CPPFLAGS) -I$(ROOT)/src $(C_COMPILER_FLAGS) $(LTO_FLAGS) -UNDEBUG $(TEST_DEFINES) -MMD -MP -c $< -o $@

generated/objects/production/%.o: $(ROOT)/src/%.c Makefile $(ROOT)/tests/leaf.mk
>    @mkdir -p $(dir $@)
>    $(C_COMPILER) $(CPPFLAGS) -I$(ROOT)/src $(C_COMPILER_FLAGS) $(LTO_FLAGS) -UNDEBUG $(TEST_DEFINES) -MMD -MP -c $< -o $@

-include $(TEST_DEPS)

run: build
>    ./generated/test

clean:
>    $(RM) -r generated
