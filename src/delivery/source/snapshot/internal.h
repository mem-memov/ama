/** @file
 * @brief Decode explicit captured source inputs without filesystem discovery or execution.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates prepared data from project effects;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves explicit original analysis identities;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * checks captured staged bytes without probing future paths;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * byte values and failure coordinates;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns all failed scratch.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires package metadata;
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) keeps exact registry identities.
 */
#ifndef AMA_DELIVERY_SOURCE_SNAPSHOT_INTERNAL_H
#define AMA_DELIVERY_SOURCE_SNAPSHOT_INTERNAL_H
#include "snapshot.h"
#include "source/json/schema/schema.h"
/** Parse scratch is independent of public result storage under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct { DeliverySourceSnapshot *owner; SourceArena *scratch; } DeliverySnapshotReader;
/** Report one original request field and static explanation under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
SourceJsonStatus delivery_snapshot_error(DeliverySnapshotReader *, const SourceJsonValue *, const char *);
/** Retain allocation failure at its request value under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
SourceJsonStatus delivery_snapshot_memory(DeliverySnapshotReader *, const SourceJsonValue *);
/** Decode exact selected version contexts under
 * [PKG-03](../../../../spec/project/packages.md#pkg-03).
 */
SourceJsonStatus delivery_snapshot_versions(DeliverySnapshotReader *, const SourceJsonValue *);
/** Decode explicit module identities and captured source/companion rows under
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
SourceJsonStatus delivery_snapshot_entries(DeliverySnapshotReader *, const SourceJsonValue *);
#endif
