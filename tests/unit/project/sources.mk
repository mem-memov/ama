# @file
# @brief Compose the explicit production dependencies for this component test boundary.
# Contract: ../../../docs/implementation/library-tests-and-build.md; generated output is never a maintained input.
include $(ROOT)/tests/unit/parser/frontend.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/project)) \
    source/io/load.c source/io/buffer/plan.c source/io/buffer/append.c \
    source/path/exact.c source/path/logical.c source/module/catalog.c \
    source/module/resolve/probe.c source/module/resolve/logical.c \
    source/module/resolve/source.c source/module/resolve/override.c source/module/resolve/snapshot.c source/module/resolve/configuration-snapshot.c source/module/resolve/native-snapshot.c source/module/resolve/native.c source/module/resolve/companion.c source/module/resolve/configuration.c source/module/resolve/alias-snapshot.c source/module/resolve/directory.c \
    source/module/scan/entries.c source/module/scan/walk.c source/module/scan/visit.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/module/packages)) \
    diagnostics/adapters/module.c

SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/path/normalize))
