/** @file
 * @brief Inspect admitted context state through declared storage and explicit immutable source metadata.
 * [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03) forbids fabricated invocation history;
 * [PAR-02](../../../spec/language/async-and-parallelism.md#par-02) requires exclusive context access.
 */
#ifndef AMA_RUNTIME_DEBUGGER_H
#define AMA_RUNTIME_DEBUGGER_H
#include "runtime/context/context.h"
#include "runtime/program/lists/traversal/traversal.h"

/** Immutable original source names for one declared field; array order is the compiler's storage ordinal. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct { const char *name; uint64_t operation; } AmaDebugField;
/** Immutable type and lexical-binding names, independently of native payload representation. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    uint64_t type_id;
    const char *name;
    const AmaDebugField *fields;
    size_t field_count;
    const char *const *bindings;
    size_t binding_count;
} AmaDebugType;
/** Shared declarations have source names but no instance identity or service controls. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct { const char *owner; const char *name; uint64_t operation; } AmaDebugConstant;
/** Borrowed compiler catalog. Exact source-map pointer/count authenticate the matching program schema;
 * C providers must supply truthful immutable metadata retained through the session.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    const AmaProgramLocation *locations;
    size_t location_count;
    const AmaDebugType *types;
    size_t type_count;
    const AmaDebugConstant *constants;
    size_t constant_count;
} AmaDebugCatalog;

/** Explicit typed preview formats; native resource layout is not a generic byte view. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef enum { AMA_DEBUG_BOOLEAN, AMA_DEBUG_INTEGER, AMA_DEBUG_TEXT, AMA_DEBUG_BYTES } AmaDebugValueKind;
/** Temporary native value observation. Integer magnitude uses declared byte order and sign;
 * bytes remain borrowed only during the callback. Readers publish canonical live values.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    AmaDebugValueKind kind;
    bool flag;
    bool negative;
    bool little_endian;
    AmaByteView bytes;
} AmaDebugNativeValue;
/** Returning false stops a synchronous preview; copy data before the callback returns. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef bool (*AmaDebugNativeVisitor)(void *, const AmaDebugNativeValue *);
/** A trusted type-specific read adapter is explicit host authority, not inferred from a native State layout.
 * It reads only its declared type, retains no payload borrow and invokes no Ama execution.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    uint64_t type_id;
    AmaStatus (*read)(const void *, AmaDebugNativeVisitor, void *);
} AmaDebugNativeReader;

/** Linear inspection owner: holds exclusive context admission until end. All borrowed metadata
 * and callback records expire before release; never shallow-copy or mutate the contained lease/view.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    AmaContextLease lease;
    AmaContextView view;
    const AmaDebugCatalog *catalog;
    const AmaDebugNativeReader *readers;
    size_t reader_count;
    /** Active synchronous visitor frames keep end from releasing their borrowed storage. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
    size_t visits;
} AmaDebugSession;
/** Runtime state at rest. Terminated contexts retain status/control only, not reconstructed instances. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    uint64_t context;
    AmaRuntimePhase phase;
    AmaContextReference root;
    AmaContextReference current;
    AmaStatus status;
    bool failed;
    size_t type_count;
    size_t constant_count;
} AmaDebugContext;
/** One actual live instance's service/lifecycle state. Source pointers borrow the session;
 * recorded_operation is a recorded operation, not a history or an outstanding invocation.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    AmaContextReference reference;
    AmaContextReference owner;
    const char *type_name;
    AmaInstanceActivity activity;
    AmaLifetimePhase lifetime;
    AmaResumePosition resume;
    AmaReturnLink caller;
    AmaContextReference outgoing;
    uint64_t recorded_operation;
    const AmaProgramLocation *recorded_location;
    const AmaProgramLocation *resume_location;
    AmaStatus last_refusal;
    bool native_opaque;
} AmaDebugInstance;
/** Declared storage projection. Lists report acquisition/count/incarnation; scalar values report a checked-context numeric reference. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    const char *name;
    size_t field;
    AmaProgramRole role;
    size_t position;
    uint64_t type_id;
    AmaFieldKind form;
    AmaContextReference value;
    size_t count;
    uint64_t incarnation;
    bool acquired;
} AmaDebugFieldValue;
/** One current ordered occurrence, distinct from its potentially empty/stale target handle. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct { size_t position; AmaMembership membership; AmaContextReference target; } AmaDebugElement;
/** Retained lexical cursor/binding metadata; inactive local slots do not imply an existing variable value. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef struct {
    size_t scope;
    const char *binding_name;
    bool local;
    bool active;
    bool role_active;
    size_t boundary;
    AmaContextReference collection_owner;
    size_t collection_field;
    uint64_t incarnation;
    AmaMembership current;
    AmaContextReference target;
    size_t binding_roles;
    bool associated;
    AmaContextReference value;
    uint64_t header;
    uint64_t repeat;
} AmaDebugCursor;
/** Synchronous instance enumeration callback; false ends the enumeration successfully. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef bool (*AmaDebugInstanceVisitor)(void *, const AmaDebugInstance *);
/** Synchronous declared-field enumeration callback; no native State bytes are exposed. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef bool (*AmaDebugFieldVisitor)(void *, const AmaDebugFieldValue *);
/** Synchronous list occurrence enumeration callback; false stops without advancing a runtime cursor. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef bool (*AmaDebugElementVisitor)(void *, const AmaDebugElement *);
/** Synchronous lexical cursor enumeration callback, separate from execution or binding activation. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
typedef bool (*AmaDebugCursorVisitor)(void *, const AmaDebugCursor *);

/** Enter an idle context exclusively into a zeroed session; rejection preserves output and context.
 * Reader arrays/catalog are borrowed through end. Active execution/construction rejects inspection.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_begin(AmaContextRegistry *, uint64_t, const AmaDebugCatalog *, const AmaDebugNativeReader *, size_t, AmaDebugSession *);
/** Consume the session before releasing its context gate; empty/forged sessions reject. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_end(AmaDebugSession *);
/** Copy admitted context status/control without reading declared or native payloads. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_context(const AmaDebugSession *, AmaDebugContext *);
/** Resolve origin/publication/lifecycle before copying actual service state, even if ordinary reads are busy. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_instance(const AmaDebugSession *, AmaContextReference, AmaDebugInstance *);
/** Stream all currently published live slots without creating/acquiring instances or reading native payload. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_instances(AmaDebugSession *, AmaDebugInstanceVisitor, void *);
/** Enumerate declared interface/persistent fields under the session's exclusive admission. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_fields(AmaDebugSession *, AmaContextReference, AmaDebugFieldVisitor, void *);
/** Enumerate actual current membership order without running methods or modifying a saved cursor. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_elements(AmaDebugSession *, AmaContextReference, size_t, AmaDebugElementVisitor, void *);
/** Enumerate compiler-defined cursor/binding slots, keeping inactive locals explicitly absent. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_cursors(AmaDebugSession *, AmaContextReference, AmaDebugCursorVisitor, void *);
/** Invoke only an explicitly registered read adapter for an available instance; absent readers expose no native bytes. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_native(AmaDebugSession *, AmaContextReference, AmaDebugNativeVisitor, void *);
#endif
