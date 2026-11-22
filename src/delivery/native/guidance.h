/** @file
 * @brief Retain optional guidance for every version represented in emitted types.
 * [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * reports the relevant effective package even when its types are ordinary Ama
 * modules and no native companion belongs to that package.
 */
#ifndef AMA_DELIVERY_VERSION_GUIDANCE_H
#define AMA_DELIVERY_VERSION_GUIDANCE_H
#include "capture.h"
/** Arena-owned version row borrowing the catalog and an existing or newly
 * captured README snapshot under
 * [BUILD-05](../../../spec/implementation/application-builds.md#build-05).
 */
typedef struct DeliveryVersionGuidance {
    struct DeliveryVersionGuidance *next;
    const ModuleVersion *version;
    const DeliveryFileSnapshot *readme;
} DeliveryVersionGuidance;
/** Select distinct actual planned-type versions in logical order. Reuse the
 * identical native capture's README observation, otherwise capture exactly one
 * optional document per version. Standard origins contribute no invented version.
 * Output changes only on success; both owners and frontend must outlive it.
 * Unavailable guidance is data; allocation failure leaves destructible scratch.
 * [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * requires optional guidance independently of native instruction presence.
 */
bool delivery_version_guidance(SourceArena *, SourceManager *, const CompilerFrontend *,
    const DeliveryNativeCapture *, const DeliveryVersionGuidance **, Diagnostic *);
#endif
