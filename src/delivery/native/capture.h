/** @file
 * @brief Capture selected native instruction and package-guidance origins.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01)
 * selects companions from actual concrete origins;
 * [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * keeps exact effective-version guidance optional.
 */
#ifndef AMA_DELIVERY_NATIVE_CAPTURE_H
#define AMA_DELIVERY_NATIVE_CAPTURE_H
#include "closure.h"
#include "files/snapshot.h"
#include "resources.h"
/** Owned relation borrowing its concrete origin and effective version under
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01).
 * Standard native modules have no package README and leave guidance null.
 */
typedef struct DeliveryNativeCapture {
    struct DeliveryNativeCapture *next;
    const DeliveryNativeOrigin *origin;
    DeliveryFileSnapshot cmake;
    const DeliveryFileSnapshot *guidance;
    const DeliveryNativeResource *resources;
} DeliveryNativeCapture;
/** Capture each supplied ordered origin once. All records/path scratch belong
 * to arena; raw bytes belong to sources. The origin/frontend owners outlive the
 * result. README observations share one snapshot for each actual version owner.
 * A supplied unavailable CMake file refuses; unavailable guidance remains data.
 * No CMake execution or alternate package-copy lookup occurs. Failure changes
 * no caller output; partial allocations remain destructible by both owners.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01) defines
 * instruction severity; [BUILD-05](../../../spec/implementation/application-builds.md#build-05)
 * defines guidance severity. Input origins must come from delivery_native_origins.
 */
bool delivery_native_capture(SourceArena *, SourceManager *, const DeliveryNativeOrigin *,
    const DeliveryNativeCapture **, Diagnostic *);
#endif
