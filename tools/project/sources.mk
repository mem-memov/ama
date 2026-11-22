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
    $(call source_files,$(ROOT)/src/source/io))

# Shared response/request data readers own no compiler, Ama parser or mutation state.
PROJECT_TOOL_SHARED_SOURCES += $(patsubst $(ROOT)/src/%,%,$(call source_files,$(ROOT)/src/source/json) \
    $(call source_files,$(ROOT)/src/delivery/source/response))
