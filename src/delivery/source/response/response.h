/** @file
 * @brief Decode source-domain replies into independent plain captured data and dependency records.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps consumers independent of compiler owners;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves exact identity and original binding facts;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains bytes and bounded source sites;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns every partial result.
 */
#ifndef AMA_DELIVERY_SOURCE_RESPONSE_H
#define AMA_DELIVERY_SOURCE_RESPONSE_H
#include "../records.h"
#include "source/json/capture/capture.h"
/** One reported version observation, not authority to replace local capture.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires later equality with the prepared input version set.
 */
typedef struct DeliveryResponseVersion {
    struct DeliveryResponseVersion *next;
    const char *identity, *directory;
    const SourceText *configuration;
} DeliveryResponseVersion;
/** Exact source and optional configuration/native observations. False captured
 * flags mean unobserved, distinct from true with absent data under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
typedef struct DeliveryResponseSource {
    struct DeliveryResponseSource *next;
    const char *identity;
    const SourceText *source, *configuration;
    bool configuration_captured, native_captured;
    const char *native_body, *native_header;
    const SourceText *native_abi;
} DeliveryResponseSource;
/** Plain result owner with no live compiler reference. SourceSpan edges reference
 * capture-owned SourceText records. valid means closed schema/graph coherence,
 * not successful child completion, equality to local inputs or executable proof.
 * Names/source bytes survive original JSON/syntax teardown; failure diagnostics
 * borrow original JSON under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    SourceArena arena;
    SourceJsonCapture capture;
    const DeliveryResponseVersion *versions;
    const DeliveryResponseSource *sources;
    const DeliverySourceDefinition *definitions;
    SourceJsonIssue issue;
    SourceJsonStatus status;
    bool attempted, valid;
} DeliverySourceResponse;
/** Begin one decode under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
void delivery_source_response_init(DeliverySourceResponse *, SourceAllocator);
/** Retire graph/observations before owned bytes under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void delivery_source_response_destroy(DeliverySourceResponse *);
/** Decode one complete ama.source-domain.v1 reply. Require exact field sets,
 * canonical unique source/version/definition identities, known active targets,
 * bounded original owner-source sites and consistent observation flags. No I/O,
 * Ama parsing, process invocation or code execution occurs. All failures keep
 * valid false with destructible scratch; repeated calls refuse without replacing
 * accepted data. Caller separately admits successful process completion and exact
 * equality with prepared inputs before using any graph fact under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
SourceJsonStatus delivery_source_response_read(DeliverySourceResponse *, const SourceText *);
#endif
