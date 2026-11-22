/** @file
 * @brief Admit reported observations only against their complete prepared input.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exact staged bytes rather than trusting a well-formed reply.
 */
#ifndef AMA_DELIVERY_SOURCE_RESPONSE_MATCH_H
#define AMA_DELIVERY_SOURCE_RESPONSE_MATCH_H
#include "response.h"
#include "../snapshot/snapshot.h"
/** Compare unique prepared rows against a schema-valid response, independent of
 * row order. Require all configuration observations, and match native captures
 * only where the compiler classified native source. No allocation or mutation;
 * borrowed finite acyclic lists and valid source objects must remain alive.
 * False never grants graph authority under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool delivery_source_response_matches(const DeliverySourceResponse *, const DeliverySourceSnapshotView *);
#endif
