/** @file
 * @brief Prepare the complete active captured domain for original-graph safety checks.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * includes every active definition, independent of executable reachability;
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) retains drafts
 * in the catalog without activating their incomplete bodies.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains failed
 * preparation under caller-owned arenas and frontend lifetimes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * installs explicit captured native metadata without guessing activation.
 */
#ifndef AMA_DELIVERY_SOURCE_DOMAIN_INTERNAL_H
#define AMA_DELIVERY_SOURCE_DOMAIN_INTERNAL_H
#include "domain.h"
/** Select all non-draft sources using the production parser, sort exact borrowed
 * identities, and publish the complete required set only on success. The catalog
 * retains original source bytes; captured native slots may be installed only
 * after production classification. Caller retains original diagnostic sources.
 * [SYN-04](../../../../spec/language/source-and-names.md#syn-04) defines activation;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed scratch.
 */
bool delivery_source_domain_required(CompilerFrontend *, SourceArena *, const DeliverySourceCompanions *,
    const char *const **, size_t *);
/** Require one exact captured row for a freshly classified native definition;
 * no filename/section guessing or filesystem fallback under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool delivery_source_domain_native(CompilerFrontend *, const ModuleSource *, const DeliverySourceCompanions *);
#endif
