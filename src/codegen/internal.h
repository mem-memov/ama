#ifndef AMA_PROJECT_CODEGEN_INTERNAL_H
#define AMA_PROJECT_CODEGEN_INTERNAL_H
#include "codegen.h"
#include "native/scaffold/internal.h"

size_t codegen_type_id(const NativeScaffold *, const ScaffoldType *);
const ScaffoldMember *codegen_member(const ScaffoldType *, const SemanticSymbol *);
const char *codegen_constant_name(NativeScaffold *, const SemanticConstantPlan *);
bool codegen_emit_type_setup(NativeScaffold *, FILE *);
bool codegen_emit_storage(NativeScaffold *, FILE *);
const char *codegen_indexed_name(NativeScaffold *, const char *, const char *, size_t);
bool codegen_source_definition(NativeScaffold *, FILE *, const char *, const SemanticSource *);
bool codegen_source_value(NativeScaffold *, FILE *, const char *, const SemanticSource *);
bool codegen_initializers(NativeScaffold *, FILE *, const char *, const SemanticSymbol *);
bool codegen_emit_program(NativeScaffold *, FILE *);
bool codegen_emit_sources(NativeScaffold *, FILE *);
bool codegen_emit_descriptors(NativeScaffold *, FILE *);
bool codegen_emit_header(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_launcher(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_native(NativeScaffold *, FILE *, const ScaffoldType *);
#endif
