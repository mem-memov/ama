/**
 * @file
 * @brief Provide checked storage/lifecycle primitives without a list dispatcher.
 *
 * [LIST-04](../../../spec/language/lists.md#list-04) requires typed services,
 * not an action selector or retained completion state. These C primitives keep
 * reusable construction/removal machinery and declared membership storage.
 * Service acquisition, cursors and typed method frames have dedicated tasks.
 */
#ifndef AMA_RUNTIME_LIST_H
#define AMA_RUNTIME_LIST_H
#include "runtime/objects/objects.h"

/** Borrow a live owner's exact declared membership-backed field for one operation.
 * The caller guarantees exclusive field authority, matching element type and
 * ownership; live cursor corrections remain a separate integration. This does
 * not expose a language-level service interface or retain any argument/result.
 */
typedef struct {
    AmaWorld *world;
    AmaHandle owner;
    AmaHandleList *list;
    uint64_t element_type;
    bool owning;
} AmaListStorage;

/** Insert before position (including count for append), without executing bodies.
 * Owning storage constructs/clones; reference storage copies the complete handle.
 * Failure/refusal is returned directly as status, never stored on the vector.
 * Preflight failure leaves contents intact; successful reserve may grow capacity.
 */
AmaStatus ama_list_storage_insert(AmaListStorage *, size_t position, AmaHandle source);

/** Remove an ordered range after preflighting every owned tree before cleanup.
 * Bounds are checked without overflowing. Owning destructor failure is terminal;
 * cleanup continues and preserves the primary failure. Reference targets are not
 * acquired or destroyed. Zero count is valid at the end of the vector.
 */
AmaStatus ama_list_storage_remove(AmaListStorage *, size_t position, size_t count);
#endif
