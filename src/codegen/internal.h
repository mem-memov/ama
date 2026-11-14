/**
 * @file
 * @brief Declare generated representation and label-emission phase boundaries.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Immutable semantic/IR inputs remain borrowed. Emitters may leave partial stream bytes, which artifact staging never publishes.
 * [PAR-01](../../spec/language/async-and-parallelism.md#par-01) requires the emitted
 * schemas to feed separate mutable context owners; host emission must preserve
 * the execution lease through all label and cleanup phases.
 */
#ifndef AMA_PROJECT_CODEGEN_INTERNAL_H
#define AMA_PROJECT_CODEGEN_INTERNAL_H
#include "codegen.h"
#include "native/scaffold/internal.h"

/** Reject statically valid IR whose execution adapters are not yet implemented, before staging artifacts. */
bool codegen_validate_execution(NativeScaffold *);
/**
 * Return the exact descriptor pointer ordinal starting at one, or zero if absent. This is an
 * assembly-local registry key, not a stable external type identity.
 */
size_t codegen_type_id(const NativeScaffold *, const ScaffoldType *);
/**
 * Find the exact resolved symbol pointer in a known type, or null. Caller supplies a registered
 * owner; no name matching or allocation.
 */
const ScaffoldMember *codegen_member(const ScaffoldType *, const SemanticSymbol *);
/**
 * Register a full module/category/member identity using exact source bytes and return
 * scaffold-owned generated C spelling. Preserve collision/allocation failure; no
 * transliteration.
 */
const char *codegen_constant_name(NativeScaffold *, const SemanticConstantPlan *);
/**
 * Emit checked pool initialization with real sizeof, _Alignof and offsetof, explicit
 * slot/generation defaults and typed hooks. Generated startup stops on the first failure.
 */
bool codegen_emit_type_setup(NativeScaffold *, FILE *);
/**
 * Write one C storage structure per pool-backed type: declared handles/lists followed by actual
 * native State or a reserved Ama payload byte. No default instance is emitted.
 */
bool codegen_emit_storage(NativeScaffold *, FILE *);
/**
 * Append a checked role/ordinal suffix to a validated generated base. Return arena-owned text or
 * first scaffold error; no user-name identity is inferred from this local suffix.
 */
const char *codegen_indexed_name(NativeScaffold *, const char *, const char *, size_t);
/**
 * Emit byte-array literal storage with explicit original length or resolved access descriptors.
 * Empty literals receive a C-array sentinel without changing language byte length.
 */
bool codegen_source_definition(NativeScaffold *, FILE *, const char *, const SemanticSource *);
/**
 * Write one resolved literal/local/shared descriptor with explicit acquisition/continuation
 * flags and exact nominal type ordinal; names and sources remain borrowed during emission.
 */
bool codegen_source_value(NativeScaffold *, FILE *, const char *, const SemanticSource *);
/**
 * Emit all dependent source definitions, then the ordered source array. Absence emits no array;
 * partial IO/allocation failure leaves the artifact unpublished.
 */
bool codegen_initializers(NativeScaffold *, FILE *, const char *, const SemanticSymbol *);
/**
 * Emit immutable shared schemas referring to resolved sources; context factories own
 * their mutable storage, and nonpool namespaces allocate no placeholder types.
 */
bool codegen_emit_program(NativeScaffold *, FILE *);
/**
 * Emit field initializer data and role/position/operation descriptors in declaration order,
 * followed by shared initializer sources; stream failure stays in staged output.
 */
bool codegen_emit_sources(NativeScaffold *, FILE *);
/**
 * Write immutable field kind/type/offsetof descriptors for context-owned pools from
 * validated model members. A false stream result leaves only unpublished partial bytes.
 */
bool codegen_emit_descriptors(NativeScaffold *, FILE *);
/**
 * Write runtime/typed storage/native adapters, descriptors, controls and locations into the
 * retained single-TU header. Host declarations are internal and own checked runtime contexts; shared schemas are immutable.
 */
bool codegen_emit_header(NativeScaffold *, FILE *, const ScaffoldType *);
/**
 * Write per-host atomic admission and idempotent registry disposal; independent hosts
 * share only immutable schemas and each accepted execution holds a context lease.
 */
bool codegen_emit_host_context(FILE *);
/**
 * Write explicit first-root execution and existing-root entry/resume admission; fresh
 * interfaces initialize once, and accepted failures retain the lease through cleanup.
 */
bool codegen_emit_host_initialize(NativeScaffold *, FILE *);
/**
 * Compose owner/startup checks, validate root resume eligibility, consume saved position and
 * attach the host return sentinel before label dispatch.
 */
bool codegen_emit_host_begin(NativeScaffold *, FILE *);
/**
 * Write common completion/yield/error outcomes. Report the active chain before terminal
 * disposal, clear execution admission and expose an empty root on failure.
 */
bool codegen_emit_host_finish(FILE *);
/**
 * Write one host execution function and all resolved control labels. Emit saved-position/type
 * and return-selector switches with invalid-selector failure paths; no recursive Ama C function
 * calls.
 */
bool codegen_emit_body(NativeScaffold *, FILE *, const ScaffoldType *);
/**
 * Write standalone main that enters once, diagnoses an unhandled root yield, always disposes and
 * returns execution/cleanup status.
 */
bool codegen_emit_launcher(NativeScaffold *, FILE *, const ScaffoldType *);
/**
 * Write a supported UTF-8 JSON compilation database argv for the launcher, explicit
 * flags/includes and non-LTO compilation. The output is informational tooling, not shell text.
 */
bool codegen_emit_tooling(NativeScaffold *, FILE *, const ScaffoldType *);
/**
 * Include the independent interface, require exact entry signature compatibility before the
 * authored body, then emit present lifecycle/conversion adapters.
 */
bool codegen_emit_native(NativeScaffold *, FILE *, const ScaffoldType *);
/**
 * Emit opaque declared reference/list locations, then scopes/binders in exact eager-layout
 * ordinal order. Reference cycles do not recursively expand storage.
 */
bool codegen_emit_views(NativeScaffold *, FILE *);
/**
 * Emit a finite resolved nested index chain from inside out so each referenced descriptor
 * already exists. No C recursion; repeated exact rescans cost O(depth squared).
 */
bool codegen_access_definition(NativeScaffold *, FILE *, const char *, const SemanticAccess *);
/**
 * Return the registered per-node generated control name for exact type/node ownership; retain
 * scaffold-owned name storage through emission.
 */
const char *codegen_control_name(NativeScaffold *, const ScaffoldType *, const ControlNode *);
/**
 * Emit resolved immutable invocation/transfer data needed by control labels; required operation
 * sites must match exactly rather than fallback to another phase.
 */
bool codegen_emit_controls(NativeScaffold *, FILE *);
/**
 * Write one validated IR control label and its exact successor/failure behavior. Body execution
 * is generated labels, not runtime helper dispatch.
 */
bool codegen_emit_control(NativeScaffold *, FILE *, const ScaffoldType *, const ControlNode *);
/** Emit a selected acquisition/release or bounded chain abandonment from exact
 * planned frame ancestry; no source resolution or C-stack activation is used.
 */
bool codegen_emit_tree_control(FILE *, const ScaffoldType *, const ControlNode *, const char *name);
/**
 * Write the exact typed native entry use inside the generated execution function; supported
 * compilation and symbol verification enforce inlining.
 */
bool codegen_emit_native_entry(FILE *, const ScaffoldType *);
/**
 * Find a declaration by exact symbol identity after the allocated control-selector range. Check
 * increment exhaustion and return zero on missing/failed selection; repeated scans are exact.
 */
size_t codegen_declaration_selector(NativeScaffold *, const SemanticSymbol *);
/**
 * Emit control and declaration locations in model order, preserving exact operation selectors
 * and source-backed roles. The generated table borrows immutable compiled strings.
 */
bool codegen_emit_locations(NativeScaffold *, FILE *);
#endif
