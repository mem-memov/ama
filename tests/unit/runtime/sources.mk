include $(ROOT)/make/sources.mk
SOURCES := $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/runtime))
SOURCES += source/text/manager.c source/text/lines.c source/text/view.c source/memory/arena.c
