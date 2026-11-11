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
bool codegen_emit_host_begin(NativeScaffold *, FILE *);
bool codegen_emit_host_finish(FILE *);
bool codegen_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_launcher(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_native(NativeScaffold *, FILE *, const ScaffoldType *);
bool codegen_emit_views(NativeScaffold *, FILE *);
bool codegen_access_definition(NativeScaffold *, FILE *, const char *, const SemanticAccess *);
const char *codegen_control_name(NativeScaffold *, const ScaffoldType *, const ControlNode *);
bool codegen_emit_controls(NativeScaffold *, FILE *);
bool codegen_emit_control(NativeScaffold *, FILE *, const ScaffoldType *, const ControlNode *);
bool codegen_emit_native_entry(NativeScaffold *, FILE *, const ScaffoldType *);
size_t codegen_declaration_selector(NativeScaffold *, const SemanticSymbol *);
bool codegen_emit_locations(NativeScaffold *, FILE *);
#endif
