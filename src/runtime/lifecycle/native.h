/**
 * @file
 * @brief Separate native construction credit from validation of its status transport.
 *
 * [LIFE-02](../../../spec/implementation/native-interface.md#life-02) gives one
 * destructor obligation to a successful native constructor. Under
 * [ERROR-03](../../../spec/implementation/diagnostics.md#error-03), raw code zero
 * denotes success even if another transport field is malformed. Credit that
 * constructed payload before returning the checked technical failure for unwind.
 */
#ifndef AMA_RUNTIME_LIFECYCLE_NATIVE_H
#define AMA_RUNTIME_LIFECYCLE_NATIVE_H
#include "lifetime.h"

/** Finish a valid constructing ledger with its entire owned-child prefix ready.
 * Nonzero native code creates no credit and leaves partial native cleanup to the
 * hook. Zero code creates the normal lifecycle credit before malformed transport
 * can trigger rollback. This operation allocates nothing and publishes no identity.
 * Component contract: [LIFE-02](../../../spec/implementation/native-interface.md#life-02), [ERROR-03](../../../spec/implementation/diagnostics.md#error-03). */
AmaStatus ama_lifetime_finish_native(AmaLifetime *, bool native_constructed, AmaStatus);
#endif
