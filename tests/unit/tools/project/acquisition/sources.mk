# @file
# @brief Compose common parsed-scope and normalized-identity foundations for acquisition planners.
# Contract: ../../../../../docs/implementation/acquisition-plan.md.
include $(ROOT)/make/sources.mk
include $(ROOT)/tools/project/sources.mk
SOURCES := $(PROJECT_TOOL_SHARED_SOURCES) $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/path/normalize)) \
    source/path/syntax.c source/text/manager.c source/text/lines.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/configuration/document) \
    $(call source_files,$(ROOT)/src/source/configuration/records) $(call source_files,$(ROOT)/src/source/configuration/lines))
