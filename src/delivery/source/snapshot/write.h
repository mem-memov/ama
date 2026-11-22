/** @file
 * @brief Serialize an explicit borrowed source capture table without inventing an accepted decoder owner.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps capture preparation outside compilation;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * byte/identity values; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete captured companion authority.
 */
#ifndef AMA_DELIVERY_SOURCE_SNAPSHOT_WRITE_H
#define AMA_DELIVERY_SOURCE_SNAPSHOT_WRITE_H
#include <stdio.h>
#include "snapshot.h"
/** Write/flush ama.source-snapshot.v1 from explicit borrowed rows/versions and
 * one unconditional companion row per source. Require canonical selected
 * identity/version authority and coherent source captures prepared by the caller;
 * no schema-valid flag or source role is guessed. Refuse incomplete/ambiguous
 * row authority, invalid basic path/metadata storage and stream failure.
 * Output may be partial on encoding/write failure and is unusable unless true.
 * Borrow inputs/stream synchronously, never close or mutate them, and perform
 * no project discovery, native execution or source writes. This is data
 * serialization, not language acceptance, under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool delivery_source_snapshot_write(FILE *, const DeliverySourceSnapshotView *);
#endif
