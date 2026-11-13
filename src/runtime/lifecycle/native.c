/**
 * @file
 * @brief Credit successful native construction before checked-status failure cleanup.
 *
 * [LIFE-02](../../../spec/implementation/native-interface.md#life-02) separates
 * failed native constructors from completed payloads that still require cleanup.
 * [ERROR-03](../../../spec/implementation/diagnostics.md#error-03) makes raw code
 * authoritative; validation must not lose an already acquired resource obligation.
 */
#include "native.h"
#include "errors.h"

/** Record a raw-success payload before normalization; preserve a malformed status
 * as the primary failure. The caller supplies a consistent completed child prefix.
 * Publication is a later phase, so checked failure always retains private ownership.
 */
AmaStatus ama_lifetime_finish_native(AmaLifetime *lifetime, bool native_constructed, AmaStatus status) {
    if (status.code != AMA_OK) { return ama_status_checked(status); }
    AmaStatus finished = ama_lifetime_finish(lifetime, native_constructed);
    return ama_cleanup_primary(ama_status_checked(status), finished);
}
