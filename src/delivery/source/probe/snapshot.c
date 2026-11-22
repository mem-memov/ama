/** @file
 * @brief Reuse captured registry/native preparation before independent normal entry inference.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves captured future-path authority;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) requires typed entry proof rather than formal counts.
 */
#include "snapshot.h"
#include "../domain/internal.h"
DeliveryEntryState delivery_source_snapshot_entry(CompilerFrontend *frontend, SourceArena *arena,
    const DeliverySourceSnapshot *snapshot, const char *entry) {
    if (!arena || !entry || !delivery_source_snapshot_catalog(frontend, snapshot)) { return DELIVERY_ENTRY_INVALID; }
    const char *const *required = NULL; size_t count = 0;
    if (!delivery_source_domain_required(frontend, arena, snapshot->companions, &required, &count)) {
        frontend->attempted = true; return DELIVERY_ENTRY_INVALID;
    }
    (void)compiler_frontend_check_entry_snapshot(frontend, entry);
    return delivery_source_entry_state(frontend);
}
