include $(ROOT)/tests/unit/project/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/semantic))
