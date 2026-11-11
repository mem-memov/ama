include $(ROOT)/tests/unit/semantic/sources.mk
SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/planning)) \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/lowering))
