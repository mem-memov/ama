/** @file
 * @brief Enumerate actual published pool slots without payload access or identity reconstruction from stale history.
 * [RT-02](../../../spec/implementation/runtime.md#rt-02) retains directory holes;
 * [REF-02](../../../spec/language/values-and-references.md#ref-02) requires exact live generations.
 */
#include <stdint.h>
#include "internal.h"
/** The caller holds exclusive ownership; metadata reads neither acquire a target nor inspect native State. */
AmaStatus ama_pool_visit(const AmaPool *pool, AmaPoolVisitor visit, void *context) {
    if (pool == NULL || visit == NULL || pool->type_id == 0 || pool->slots_per_block == 0) {
        return ama_status_make(AMA_INVALID_STATE,NULL,0);
    }
    if (pool->block_count > SIZE_MAX/pool->slots_per_block) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }
    size_t extent = pool->block_count*pool->slots_per_block;
    for (size_t slot = 0; slot < extent; slot++) {
        const AmaPoolCell *cell = ama_pool_cell(pool,slot);
        if (cell == NULL || !cell->occupied || !cell->metadata.identity.live || cell->metadata.identity.retired) { continue; }
        const AmaSlotIdentity *identity = &cell->metadata.identity;
        if (identity->type_id != pool->type_id || identity->slot_id != slot || identity->generation == 0) {
            return ama_status_make(AMA_INVALID_STATE,NULL,0);
        }
        AmaHandle handle = {identity->type_id,identity->slot_id,identity->generation};
        if (!visit(context,handle,cell)) { break; }
    }
    return ama_status_success();
}
