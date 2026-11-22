/** @file
 * @brief Prepare the complete active captured domain for original-graph safety checks.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * includes every active definition, independent of executable reachability;
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) retains drafts
 * in the catalog without activating their incomplete bodies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation under caller-owned arenas and frontend lifetimes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * installs prepared native metadata without staged-path reads.
 */
#ifndef AMA_DELIVERY_SOURCE_DOMAIN_H
#define AMA_DELIVERY_SOURCE_DOMAIN_H
#include "../export.h"
#include "../snapshot/snapshot.h"
#include "compiler/frontend.h"
/** Check every active member of an already complete authoritative catalog
 * through the ordinary entry-free frontend, then export plain definitions/edges.
 * NULL companions uses normal catalog activation, including prepared filesystem
 * companions when not already sealed. Non-NULL companions selects captured mode:
 * after production parsing, every native definition must have exactly one row;
 * install its captured metadata without filesystem fallback. Ordinary/draft rows
 * remain opaque. Captured mode requires fresh native slots. The caller prepares
 * selected-version and module-configuration authority before this call; this
 * operation does not discover roots or certify complete physical input capture.
 * Fresh frontend required: catalog built, no project check attempted. Output
 * changes only on success; failed scratch stays arena/frontend-owned. Exported
 * spans borrow frontend source owners. Failed preparation consumes the attempt
 * and retains original diagnostics. No test/native/Ama execution, source mutation
 * or executable entry is introduced. Open templates receive no invented types;
 * the frontend executable-valid flag remains false even after success.
 * For N sources and C companion rows, selection uses O(N log N) name comparisons
 * plus O(N*C) captured-row comparisons and O(N) pointer storage/one transient AST;
 * parser/check/export costs are additional.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires the whole domain; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps staged companion authority explicit; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * fixes successful/failed ownership.
 */
bool delivery_source_domain(CompilerFrontend *, SourceArena *, const DeliverySourceCompanions *,
    const DeliverySourceDefinition **);
/** Install a complete decoded captured input in a fresh empty frontend catalog,
 * then check the whole domain with mandatory captured companion authority.
 * No source/configuration/native path is probed. Snapshot data remains borrowed
 * through checking and failed diagnostic consumption; successful frontend source
 * copies and exported facts are independent of the input owner. Preparation
 * failure consumes the frontend attempt and retains destructible catalog state.
 * Output changes only after complete ordinary checking/export. This does not
 * seal physical inputs or validate execution/test-entry admission.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * validates proposed bytes before publication; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * fixes successful/failed ownership and prevents reuse of partial catalogs.
 */
bool delivery_source_domain_snapshot(CompilerFrontend *, SourceArena *, const DeliverySourceSnapshot *,
    const DeliverySourceDefinition **);

#endif
