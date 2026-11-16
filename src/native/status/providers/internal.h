/**
 * @file
 * @brief Separate exact provider key ordering, in-place sorting and bounded ordinal encoding.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) requires
 * deterministic collision-free native codes; these compiler-only phases neither
 * inspect message wording nor allocate runtime error state.
 */
#ifndef AMA_NATIVE_STATUS_PROVIDERS_INTERNAL_H
#define AMA_NATIVE_STATUS_PROVIDERS_INTERNAL_H
#include "providers.h"

/** Compare valid immutable complete encodings lexicographically, then by byte length. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04). */
int native_status_provider_compare(const IdentityName *, const IdentityName *);
/** Sort a private row array by exact key using bounded in-place heap sort. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04). */
void native_status_providers_order(NativeStatusProvider *, size_t);
/** Encode zero-based provider ordinal as nonzero namespace; exhaustion preserves output. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04). */
AmaStatus native_status_provider_ordinal(size_t, uint16_t *);
#endif
