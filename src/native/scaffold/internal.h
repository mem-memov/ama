#ifndef AMA_NATIVE_SCAFFOLD_INTERNAL_H
#define AMA_NATIVE_SCAFFOLD_INTERNAL_H

#include "scaffold.h"
#include "native/namespace/namespace.h"

typedef bool (*ScaffoldEmitter)(NativeScaffold *, FILE *, const ScaffoldType *);
void *scaffold_allocate(NativeScaffold *scaffold, size_t size);
bool scaffold_error(NativeScaffold *scaffold, DiagnosticCode code, const char *message, const char *path);
const char *scaffold_join(NativeScaffold *scaffold, const char *left, const char *separator, const char *right);
const char *scaffold_absolute(NativeScaffold *scaffold, const char *path);
bool scaffold_paths(NativeScaffold *scaffold);
bool scaffold_hooks(NativeScaffold *scaffold, const ScaffoldType *type);
bool scaffold_hook_first(const ScaffoldType *type, AbiOperation operation);
bool scaffold_model(NativeScaffold *scaffold);
bool scaffold_artifact(NativeScaffold *scaffold, const char *path, bool authored,
    ScaffoldEmitter emitter, const ScaffoldType *type);
bool scaffold_emit_all(NativeScaffold *scaffold);
bool scaffold_emit_types(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_interface(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_context(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_end(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_payload(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_abi(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_test(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
bool scaffold_emit_signature(FILE *output, const ScaffoldType *type, AbiOperation operation, SourceSpan function);
bool scaffold_include(FILE *output, const char *path);
bool scaffold_manifest(NativeScaffold *scaffold);
bool scaffold_owned_output(NativeScaffold *scaffold, const ScaffoldArtifact *artifact);
bool scaffold_json(FILE *output, const char *text);

#endif
