/** @file
 * @brief Retire fresh-state borrowers before releasing exclusive source access.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * owns failed prefixes; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps exclusion until publication or rollback has finished.
 */
#include <string.h>
#include "guard.h"
void source_transaction_guard_init(SourceTransactionGuard *guard, SourceAllocator allocator) {
    memset(guard, 0, sizeof(*guard)); source_arena_init(&guard->arena, allocator);
    source_transaction_lease_init(&guard->lease, allocator); source_observed_image_init(&guard->fresh, allocator);
}
int source_transaction_guard_destroy(SourceTransactionGuard *guard) {
    source_observed_image_destroy(&guard->fresh);
    int error = source_transaction_lease_destroy(&guard->lease);
    source_arena_destroy(&guard->arena); memset(guard, 0, sizeof(*guard)); return error;
}
