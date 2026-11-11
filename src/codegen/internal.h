#ifndef AMA_PROJECT_CODEGEN_INTERNAL_H
#define AMA_PROJECT_CODEGEN_INTERNAL_H
#include "codegen.h"
#include "native/scaffold/internal.h"

size_t codegen_type_id(const NativeScaffold *, const ScaffoldType *);
const ScaffoldMember *codegen_member(const ScaffoldType *, const SemanticSymbol *);
const char *codegen_constant_name(NativeScaffold *, const SemanticConstantPlan *);
bool codegen_emit_type_setup(NativeScaffold *, FILE *);
bool codegen_emit_storage(NativeScaffold *, FILE *);
bool codegen_emit_sources(NativeScaffold *, FILE *);
bool codegen_emit_descriptors(NativeScaffold *, FILE *);
bool codegen_emit_header(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_launcher(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_native(NativeScaffold *, FILE *, const ScaffoldType *);
#endif
