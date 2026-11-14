/** @file
 * @brief Share exact host-control admission and lifecycle ownership inside the C boundary.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) requires
 * rejection before mutable Ama state access and forbids numeric status heuristics.
 */
#ifndef AMA_RUNTIME_LIBRARY_INTERNAL_H
#define AMA_RUNTIME_LIBRARY_INTERNAL_H
#include "lifecycle.h"
/** One completely published explicit root; IDs are never reused within its context. */
typedef struct AmaLibraryRoot {
    struct AmaLibraryRoot *next;
    uint64_t identity, exported;
    AmaHandle handle;
} AmaLibraryRoot;
/** Administrative host record. A core lease pins this record until API access ends. */
struct AmaLibraryContextRecord {
    AmaLibraryContextRecord *next;
    uint64_t identity, next_instance;
    bool exhausted;
    AmaLibraryRoot *roots;
};
/** Linear library admission owner; its core lease excludes disposal and other executors. */
typedef struct {
    AmaContextLease lease;
    AmaContextView view;
    AmaLibraryContextRecord *record;
} AmaLibraryAccess;
/** Lock only a finite host-record critical section; execute no allocator/hook while held. */
void ama_library_lock(AmaLibraryService *);
/** Publish metadata writes and release the short service gate. */
void ama_library_unlock(AmaLibraryService *);
/** Acquire an exact checked context and its core gate; failure changes no private state. */
AmaLibraryResult ama_library_enter(AmaLibraryService *, AmaLibraryContext, AmaLibraryAccess *);
/** Release one held library/core admission; no borrowed view remains usable afterward. */
void ama_library_leave(AmaLibraryAccess *);
/** Build a plain independent API result; native status values never choose provenance. */
AmaLibraryResult ama_library_result(AmaLibraryCode, AmaStatus);
/** Release host root mappings after core cleanup has consumed their Ama instances. */
void ama_library_forget_roots(AmaLibraryService *, AmaLibraryContextRecord *);
/** Fail an already admitted healthy context and invalidate every host instance mapping. */
AmaLibraryResult ama_library_fail(AmaLibraryService *, AmaLibraryAccess *, AmaStatus);
#endif
