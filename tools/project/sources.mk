# @file
# @brief Share narrow immutable source views, owned allocation and canonical identity helpers.
# Contract: ../../docs/implementation/project-tool.md; PROJECT-01 forbids dispatcher sharing.
PROJECT_TOOL_SHARED_SOURCES := source/path/logical.c source/path/version.c source/unicode/decode.c \
    source/unicode/identifier.c source/unicode/word.c source/unicode/name.c source/text/view.c source/memory/arena.c

# Exact source-directory/byte capture is a neutral preparation foundation. Keep
# parser, semantics, planning, compiler dispatcher and source-export producers out
# of this executable; tools consume only their plain checked record schema.
PROJECT_TOOL_CAPTURE_SOURCES := source/text/manager.c source/text/lines.c source/path/exact.c \
    delivery/native/files/capture.c \
    $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/module) \
    $(call source_files,$(ROOT)/src/source/io) \
    $(call source_files,$(ROOT)/src/source/path/physical))

# Shared response/request data readers own no compiler, Ama parser or mutation state.
PROJECT_TOOL_SHARED_SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json) \
    $(call source_files,$(ROOT)/src/delivery/source/response))

# Shared neutral configuration and actual-domain preparation serve project scopes.
# These components own no Ama parser, compiler dispatch or execution state.
PROJECT_TOOL_SHARED_SOURCES += source/path/syntax.c $(patsubst $(ROOT)/src/%,%,\
    $(call source_files,$(ROOT)/src/source/configuration) \
    $(call source_files,$(ROOT)/src/source/path/normalize))
PROJECT_TOOL_SHARED_SOURCES := $(sort $(PROJECT_TOOL_SHARED_SOURCES))

# Lossless image journal records share only neutral JSON scalar writers; no
# compiler dispatcher, schema producer or execution state is linked here.
PROJECT_TOOL_RECORD_SOURCES := $(patsubst $(ROOT)/src/%,%,\
    $(call source_files,$(ROOT)/src/tools/output/json))
