# Link filesystem phases without frontend, semantic, planning or scaffold-model code.
ROOT := $(abspath ../../../../..)
include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/native/scaffold/output/publication))
SOURCES += source/memory/arena.c source/text/manager.c source/text/lines.c source/text/view.c
include $(ROOT)/tests/leaf.mk
