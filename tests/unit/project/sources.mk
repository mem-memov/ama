include $(ROOT)/tests/unit/parser/frontend.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/project)) \
    source/io/load.c source/path/exact.c source/module/catalog.c \
    source/module/resolve/probe.c source/module/resolve/logical.c \
    source/module/resolve/source.c source/module/resolve/directory.c \
    source/module/scan/entries.c source/module/scan/walk.c source/module/scan/visit.c \
    diagnostics/adapters/module.c
