/** @file
 * @brief Decode source-domain replies into independent plain captured data and dependency records.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps consumers independent of compiler owners;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact identity and original binding facts;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains bytes and bounded source sites;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns every partial result.
 */
#ifndef AMA_DELIVERY_SOURCE_RESPONSE_INTERNAL_H
#define AMA_DELIVERY_SOURCE_RESPONSE_INTERNAL_H
#include "response.h"
#include "source/json/schema/schema.h"
/** Temporary syntax scratch stays separate from result ownership under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct { DeliverySourceResponse *owner; SourceArena *scratch; } DeliveryResponseReader;
/** Preserve original response field diagnostics under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
SourceJsonStatus delivery_response_error(DeliveryResponseReader *, const SourceJsonValue *, const char *);
/** Preserve allocation failure at its original field under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
SourceJsonStatus delivery_response_memory(DeliveryResponseReader *, const SourceJsonValue *);
/** Decode unique version observations under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
SourceJsonStatus delivery_response_versions(DeliveryResponseReader *, const SourceJsonValue *);
/** Decode unique source/capture observations under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
SourceJsonStatus delivery_response_sources(DeliveryResponseReader *, const SourceJsonValue *);
/** Decode byte-sorted definitions and owner-backed dependency sites under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
SourceJsonStatus delivery_response_definitions(DeliveryResponseReader *, const SourceJsonValue *);
/** Find one already validated exact source identity, never a prefix or fallback
 * under [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
const DeliveryResponseSource *delivery_response_source(const DeliverySourceResponse *, const char *);
#endif
