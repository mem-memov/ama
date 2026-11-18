# @file
# @brief Share the explicit frontend translation units after a consumer's planning dependencies.
# Contract: ../../../docs/implementation/native-static-admission.md.
# Include after planning/sources.mk; inspection and command-specific adapters remain separate.
SOURCES += compiler/admission.c compiler/frontend.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
