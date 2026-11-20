# @file
# @brief Build and run exact package-selection and measured directory-I/O failure cases.
# Contract: ../../../../../docs/implementation/versioned-source.md; generated output is never a maintained input.
ROOT := $(abspath ../../../../..)
LOCAL_SOURCES := main.c support/io.c
LINKER_FLAGS += -Wl,--wrap=opendir -Wl,--wrap=readdir -Wl,--wrap=closedir -Wl,--wrap=lstat
SOURCES := source/text/manager.c source/text/lines.c source/text/view.c \
    source/io/load.c source/io/buffer/plan.c source/io/buffer/append.c \
    source/memory/arena.c source/unicode/decode.c source/unicode/identifier.c source/unicode/word.c source/unicode/name.c \
    source/path/exact.c source/module/catalog.c source/module/resolve/probe.c source/module/resolve/logical.c \
    source/module/resolve/source.c source/module/resolve/override.c source/module/resolve/snapshot.c source/module/resolve/native-snapshot.c source/module/resolve/native.c source/module/resolve/directory.c source/module/scan/entries.c \
    source/module/scan/walk.c source/module/scan/visit.c \
    source/path/version.c source/module/packages/select.c source/module/packages/physical.c source/module/packages/discover.c \
    source/module/packages/queue.c source/module/packages/registry.c source/module/packages/visit.c \
    source/module/packages/index.c source/path/logical.c
include ../../../../leaf.mk
