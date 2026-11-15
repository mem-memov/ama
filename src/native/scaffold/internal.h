/**
 * @file
 * @brief Declare native model, C declaration emission and artifact-ownership phase contracts.
 *
 * [CABI-06](../../../spec/implementation/native-interface.md#cabi-06)
 * [PROTO-03](../../../spec/implementation/native-interface.md#proto-03) binds
 * only the exact standard integer size bridge; [ERROR-04](../../../spec/implementation/diagnostics.md#error-04)
 * plans complete native provider keys before emission.
 * Native authoring headers stand alone; publication IO uses its separate narrow frontend-free phase interface.
 */
#ifndef AMA_NATIVE_SCAFFOLD_INTERNAL_H
#define AMA_NATIVE_SCAFFOLD_INTERNAL_H

#include "scaffold.h"
#include "native/namespace/namespace.h"

/** Synchronous model-to-stream callback; artifacts own resulting bytes after successful completion. Uniform whole-model emitters may ignore the selected type. */
typedef bool (*ScaffoldEmitter)(NativeScaffold *, FILE *, const ScaffoldType *);
/** Allocate/zero scaffold-owned scratch; null records the first allocation error and leaves existing state destructible.
 */
void *scaffold_allocate(NativeScaffold *scaffold, size_t size);
/** Retain the first borrowed diagnostic message/path only and return false without allocation.
 */
bool scaffold_error(NativeScaffold *scaffold, DiagnosticCode code, const char *message, const char *path);
/** Return an arena-owned exact concatenation or preserve first allocation failure; input strings are borrowed through the call.
 */
const char *scaffold_join(NativeScaffold *scaffold, const char *left, const char *separator, const char *right);
/** Copy/compose an explicit physical path relative to the recorded working directory. Reject C-header-name syntax hazards; this is not logical module identity normalization.
 */
const char *scaffold_absolute(NativeScaffold *scaffold, const char *path);
/** Own absolute output/SDK paths and verify each required SDK header before emitting declarations; temporary allocation remains scaffold-owned on failure.
 */
bool scaffold_paths(NativeScaffold *scaffold);
/** Check explicit helper-name compatibility and reserved generated roles; lexical/function-signature usability still receives typed C verification.
 */
bool scaffold_hooks(NativeScaffold *scaffold, const ScaffoldType *type);
/** Identify the first compatible C declaration/definition for a shared helper spelling. ABI metadata records must not use this deduplication predicate.
 */
bool scaffold_hook_first(const ScaffoldType *type, AbiOperation operation);
/** Register all supported nominal views/member names, sort by exact identity and retain the selected entry. Partial records are not published as a ready scaffold.
 */
bool scaffold_model(NativeScaffold *scaffold);
/** Plan every native provider from its complete registered identity before any
 * matching header/body context is emitted; retain failed scratch for teardown.
 */
bool scaffold_providers(NativeScaffold *);
/** Recognize only the exact native standard-root integer contract. */
bool scaffold_size_bridge(const ScaffoldType *);
/** Emit an exact C type assertion using the authored from_size declaration;
 * do not invent a prototype or add an ABI metadata slot.
 */
bool scaffold_emit_bridge(FILE *, const ScaffoldType *);
/** Add the fixed bridge prototype only to a newly requested standard integer header. */
bool scaffold_emit_bridge_declaration(FILE *, const ScaffoldType *);
/** Add an unwritten fixed bridge implementation only to an explicitly generated body template. */
bool scaffold_emit_bridge_body(FILE *, const ScaffoldType *);
/** Invoke a borrowed synchronous emitter into private scratch, check all stream/size operations, then append a complete immutable artifact. Callback/model/source lifetimes outlive emission.
 */
bool scaffold_artifact(NativeScaffold *scaffold, const char *path, bool authored,
    ScaffoldEmitter emitter, const ScaffoldType *type);
/** Emit common types and each native module interface/context as complete in-memory artifacts; no filesystem publication occurs.
 */
bool scaffold_emit_declarations(NativeScaffold *);
/** Emit declarations, selected templates and optional explicitly requested authored companions, then tooling configuration.
 */
bool scaffold_emit_all(NativeScaffold *scaffold);
/** Emit all forward/representation/view definitions in prerequisite order. The uniform emitter type argument is intentionally unused for this whole-model artifact.
 */
bool scaffold_emit_types(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit one guarded standalone interface with explicit namespace begin/end and compatible declaration deduplication.
 */
bool scaffold_emit_interface(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit one module namespace plus exact field/parameter/result ordinal aliases; caller later emits matching cleanup.
 */
bool scaffold_emit_context(NativeScaffold *, FILE *, const ScaffoldType *);
/** Undefine every temporary module field/input/output alias and namespace macro, including zero-member modules.
 */
bool scaffold_emit_end(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit an interface-including fail-first entry/hook skeleton, deduplicating only compatible C definitions; no unwritten hook reports false success.
 */
bool scaffold_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit an authored native representation skeleton under its module context; persistent ordinary Ama links belong in declared Ama fields.
 */
bool scaffold_emit_payload(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit every present protocol marker/function record, including both + and - if they share one compatible C helper.
 */
bool scaffold_emit_abi(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit an explicit C/assert lifecycle test template with main/run; the author must implement native hooks before the template can pass.
 */
bool scaffold_emit_test(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit a compilation database from supported UTF-8 configuration and exact argv elements, with explicit cwd/includes/flags and no shell interpretation.
 */
bool scaffold_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
/** Emit exact native SDK role names/full keys for authoring tools. The resolved
 * model and prepared filenames are borrowed; this is not a runtime value ABI.
 */
bool scaffold_emit_model(NativeScaffold *, FILE *, const ScaffoldType *);
/** Write one valid AbiOperation signature using validated local function bytes. No type model is needed. Partial output remains unpublished on false.
 */
bool scaffold_emit_signature(FILE *output, AbiOperation operation, SourceSpan function);
/** Write a C header directive for a prevalidated representable path; caller checks final flush/close and owns any partial artifact.
 */
bool scaffold_include(FILE *output, const char *path);
/** Validate prior scoped ownership and stage the new manifest after complete normal artifacts; malformed/foreign scope requires explicit policy or failure.
 */
bool scaffold_manifest(NativeScaffold *scaffold);
/** Check exact recorded generated ownership and current content digest; never classify authored files as automatically owned. Stable filesystem/lock assumptions apply.
 */
bool scaffold_owned_output(NativeScaffold *scaffold, const ScaffoldArtifact *artifact);
/** Write one borrowed valid UTF-8 NUL-terminated string as a JSON string, escaping quotes, backslashes and controls. No allocation, input retention or shell escaping occurs.
 */
bool scaffold_json(FILE *output, const char *text);

#endif
