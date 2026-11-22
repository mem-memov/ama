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
#ifndef AMA_DELIVERY_SOURCE_SNAPSHOT_H
#define AMA_DELIVERY_SOURCE_SNAPSHOT_H
#include "../records.h"
#include "source/json/capture/capture.h"
#include "source/module/catalog.h"
/** One selected version's captured identity, directory and required metadata.
 * All fields belong to the enclosing snapshot owner under
 * [PKG-03](../../../../spec/project/packages.md#pkg-03).
 */
typedef struct DeliverySnapshotVersion {
    struct DeliverySnapshotVersion *next;
    const char *identity;
    const char *directory;
    const SourceText *configuration;
} DeliverySnapshotVersion;
/** Immutable borrowed provider view, without a decoder-owned validity flag.
 * Rows/version/companion owners outlive synchronous serialization/comparison under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef struct {
    const ModuleSnapshotEntry *entries;
    size_t count;
    const DeliverySnapshotVersion *versions;
    const DeliverySourceCompanions *companions;
} DeliverySourceSnapshotView;
/** Data-only owner: valid certifies the captured-input schema, not Ama semantics
 * or actual filesystem freshness. Arrays/capture rows belong to arena; names and
 * source/configuration/ABI byte snapshots belong to the independent capture owner. Failed prefixes are
 * unusable until destruction. Failure issue borrows the original request source;
 * successful public data survives release of the JSON request and parse scratch.
 * No compiler AST/semantic/runtime or project execution owner crosses this model
 * under [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
typedef struct {
    SourceArena arena;
    SourceJsonCapture capture;
    const ModuleSnapshotEntry *entries;
    size_t count;
    const DeliverySnapshotVersion *versions;
    const DeliverySourceCompanions *companions;
    SourceJsonIssue issue;
    SourceJsonStatus status;
    bool attempted, valid;
} DeliverySourceSnapshot;
/** Begin one data-only decode with explicit allocator lifetime under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void delivery_source_snapshot_init(DeliverySourceSnapshot *, SourceAllocator);
/** Retire result metadata before its source snapshots, including failed decode
 * prefixes under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void delivery_source_snapshot_destroy(DeliverySourceSnapshot *);
/** Decode exactly ama.source-snapshot.v1 once, with authoritative version,
 * source/configuration and unconditional companion captures. Names and source
 * bytes are copied; only failure diagnostics borrow the request source. This
 * operation does not certify physical inode authority, parse Ama, discover files
 * or execute code. The normal catalog/frontend must validate registry aliases,
 * source syntax, types and operations before any result is used as checked code.
 * No partially decoded model is valid. Empty source/version sets are permitted.
 * Work is lexical/schema bytes plus exact per-source/per-version and captured
 * path/byte coherence scans; memory is linear in retained rows/names/source bytes
 * plus temporary JSON syntax. [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires this explicit capture boundary instead of filesystem fallback.
 */
SourceJsonStatus delivery_source_snapshot_read(DeliverySourceSnapshot *, const SourceText *);
#endif
