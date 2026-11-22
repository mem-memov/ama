# @file
# @brief Share the explicit frontend translation units after a consumer's planning dependencies.
# Contract: ../../../docs/implementation/native-static-admission.md.
# Include after planning/sources.mk; inspection and command-specific adapters remain separate.
SOURCES += compiler/admission.c compiler/specialize.c compiler/roots.c compiler/frontend.c compiler/plan.c compiler/snapshot-entry.c compiler/snapshot.c compiler/resolve.c compiler/inputs.c compiler/load.c compiler/authoring/check.c
# Guarded child translation units are provided by semantic/sources.mk through planning dependencies.
