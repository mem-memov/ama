/** @file
 * @brief Retain independent entry proof owners until exact diagnostics or classification are collected.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) separates open helpers
 * from invalid definitions; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * admits captured bytes without future-path I/O;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retires scratch before borrowed sources.
 */
#include <stdlib.h>
#include "probe.h"
#include "delivery/source/probe/snapshot.h"
#include "delivery/source/probe/protocol.h"
#include "diagnostics/render/render.h"
int driver_source_domain_probe(const DeliverySourceSnapshot *snapshot, const char *entry, FILE *errors) {
    SourceAllocator allocator = source_system_allocator();
    CompilerFrontend frontend; SourceArena proof;
    compiler_frontend_init(&frontend, allocator); source_arena_init(&proof, allocator);
    DeliveryEntryState state = delivery_source_snapshot_entry(&frontend, &proof, snapshot, entry);
    int result = EXIT_FAILURE;
    if (state == DELIVERY_ENTRY_EXECUTABLE) { result = EXIT_SUCCESS; }
    else if (state == DELIVERY_ENTRY_OPEN) { result = DELIVERY_SOURCE_PROBE_OPEN_EXIT; }
    else { (void)diagnostic_render(errors, &frontend.error); }
    source_arena_destroy(&proof); compiler_frontend_destroy(&frontend); return result;
}
