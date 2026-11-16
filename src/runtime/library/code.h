/** @file
 * @brief Track exact live dependencies on one resident component's code.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * snapshots, diagnostics and detached callbacks to retain code independently of
 * contexts. This short metadata gate never invokes allocators or user code.
 */
#ifndef AMA_RUNTIME_LIBRARY_CODE_H
#define AMA_RUNTIME_LIBRARY_CODE_H
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/** Intrusive registration embedded first in a private snapshot, never a caller borrow. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
typedef struct AmaLibraryCodeOwner {
    struct AmaLibraryCodeOwner *previous, *next;
    const void *schema;
    bool registered;
} AmaLibraryCodeOwner;
/** Zero initialization is a ready resident code owner. The external loader owns
 * the barrier excluding all API calls before testing quiescence and unloading.
 * This registry cannot prevent an unrelated owner from forcibly calling dlclose.
 * Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
typedef struct {
    _Atomic bool locked;
    size_t pins;
    AmaLibraryCodeOwner *first;
} AmaLibraryCodePins;
/** Retain one diagnostic/callback dependency; false means the exact counter is exhausted. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool ama_library_code_retain(AmaLibraryCodePins *);
/** Consume one independently owned dependency; reject underflow without changes. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool ama_library_code_release(AmaLibraryCodePins *);
/** Register fresh embedded snapshot metadata and retain its code dependency. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool ama_library_code_register(AmaLibraryCodePins *, AmaLibraryCodeOwner *, const void *schema);
/** Unlink a registered embedded owner before its storage is freed. Empty is harmless. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
void ama_library_code_unregister(AmaLibraryCodePins *, AmaLibraryCodeOwner *);
/** Validate address and nominal schema using live records before dereferencing caller storage. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool ama_library_code_contains(AmaLibraryCodePins *, const void *, const void *schema);
/** Test exact outstanding dependencies while the supported loader holds its call barrier. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool ama_library_code_quiescent(AmaLibraryCodePins *);
#endif
