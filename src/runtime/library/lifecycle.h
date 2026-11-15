/** @file
 * @brief Own compiled-component context/instance controls separately from Ama values.
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) requires
 * explicit construction without bodies and quiescent disposal. These internal
 * records are never public consumer layouts or native payload references.
 */
#ifndef AMA_RUNTIME_LIBRARY_LIFECYCLE_H
#define AMA_RUNTIME_LIBRARY_LIFECYCLE_H
#include "runtime/context/context.h"
/** API admission/result categories, matching the checked consumer boundary. */
typedef enum { AMA_LIBRARY_OK, AMA_LIBRARY_INVALID_REQUEST, AMA_LIBRARY_BUSY,
    AMA_LIBRARY_REFUSED, AMA_LIBRARY_ABI_MISMATCH, AMA_LIBRARY_TECHNICAL_FAILURE } AmaLibraryCode;
/** Preserve admission provenance independently of a native status's numeric code. */
typedef struct { AmaLibraryCode code; AmaStatus status; } AmaLibraryResult;
/** Checked component/context control, not a portable Ama reference or C address to dereference. */
typedef struct { uintptr_t owner; uint64_t identity; } AmaLibraryContext;
/** Context-bound monotonically issued instance control; the local Ama handle stays private. */
typedef struct { uintptr_t owner; uint64_t context, identity; } AmaLibraryInstance;
/** Explicit compile-time export/type association, never discovered from a module basename. */
typedef struct { uint64_t identity, type; } AmaLibraryExport;
/** Opaque service-owned root and context records. */
typedef struct AmaLibraryContextRecord AmaLibraryContextRecord;
/** Explicit C service lifetime. Its short gate protects host records, never Ama
 * execution. Exclusively own this registry; do not operate its core contexts
 * through another API. Its C address stays stable until destruction.
 */
typedef struct {
    AmaContextRegistry contexts;
    _Atomic bool locked;
    /** Load-domain identity. Raw services use their stable address; generated
     * reloadable components bind a permanently retired process receipt before
     * publishing their first context. Identity is never a dereferenced handle.
     */
    _Atomic uintptr_t owner;
    SourceAllocator allocator;
    const AmaContextDefinition *definition;
    const AmaLibraryExport *exports;
    size_t export_count;
    AmaLibraryContextRecord *first;
    bool initialized;
} AmaLibraryService;
/** Initialize an empty stable service with borrowed immutable schemas and allocator.
 * No context, constant, root or body is constructed. Export IDs/types are nonzero and unique.
 */
AmaStatus ama_library_service_init(AmaLibraryService *, SourceAllocator,
    const AmaContextDefinition *, const AmaLibraryExport *, size_t);
/** End an exclusively owned service after every public context was disposed.
 * Live records refuse without effects. The service must outlive all API calls.
 */
AmaStatus ama_library_service_destroy(AmaLibraryService *);
/** Create context constants/services only. Output starts empty and stays empty on failure. */
AmaLibraryResult ama_library_context_create(AmaLibraryService *, AmaLibraryContext *);
/** Invalidate an available context before hooks; accepted cleanup empties the supplied token
 * even when cleanup returns a technical error. A busy request preserves all identities.
 */
AmaLibraryResult ama_library_context_dispose(AmaLibraryService *, AmaLibraryContext *);
/** Copy a program-lifetime immutable failure view from a healthy or failed available context.
 * The destination starts as success; the view survives context disposal while component code remains loaded.
 */
AmaLibraryResult ama_library_diagnostic(AmaLibraryService *, AmaLibraryContext, AmaStatus *);
/** Construct one explicit exported root with defaults, never its body. Accepted failure
 * terminally cleans its context; no partially created instance token is published.
 */
AmaLibraryResult ama_library_instance_create(AmaLibraryService *, AmaLibraryContext,
    uint64_t export_identity, AmaLibraryInstance *);
/** Dispose one available root without resuming it. Accepted destructor failure fails and
 * cleans the context; output empties after admission, including technical cleanup failure.
 */
AmaLibraryResult ama_library_instance_dispose(AmaLibraryService *, AmaLibraryContext,
    AmaLibraryInstance *);
#endif
