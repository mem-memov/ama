/** @file
 * @brief Declare exact protocol object, byte and scalar validation contracts.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares exact data validation without sharing compiler or mutation owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * lossless bytes and exact integer offsets without permissive conversion;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * complete/failed decoded payloads under the caller's operation arena.
 */
#ifndef AMA_SOURCE_JSON_SCHEMA_H
#define AMA_SOURCE_JSON_SCHEMA_H
#include <stdint.h>
#include "../json.h"
/** Validate exactly the named object fields, rejecting missing/unknown keys.
 * Input is a complete tree from source_json_parse; names are nonnull unique
 * static C strings. count zero permits NULL arrays. Borrow matching children
 * through the tree lifetime; outputs change only after complete validation.
 * O(count * child/key comparisons), with no allocation or field-order meaning.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_schema_fields(const SourceJsonValue *, const char *const *,
    size_t, const SourceJsonValue **, SourceJsonIssue *);
/** Decode the exact {utf8,hex} byte representation; require lowercase even hex
 * and the corresponding valid UTF-8 view or null for malformed UTF-8. Output
 * is arena-owned, length-delimited and additionally zero-terminated. Embedded
 * NUL is permitted. Output changes only on success; failed scratch stays owned.
 * O(encoded bytes) work/space; no filesystem or source interpretation occurs.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_schema_bytes(SourceArena *, const SourceJsonValue *,
    SourceJsonText *, SourceJsonIssue *);
/** Require a nonempty NUL-free JSON string and borrow its decoded terminator-
 * bounded storage. No trimming, normalization or path inference occurs.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_schema_cstring(const SourceJsonValue *, const char **, SourceJsonIssue *);
/** Require a canonical unsigned decimal STRING representable in size_t;
 * leading zeros/signs/whitespace/numeric JSON values and overflow are errors.
 * Output changes only on success; no atof/strtoul/locale fallback is used.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_schema_size(const SourceJsonValue *, size_t *, SourceJsonIssue *);
/** Require a canonical unsigned decimal STRING in the full uintmax_t range,
 * for lossless filesystem identities before target-type representability checks.
 * Output changes only on success; source spelling is never rounded or repaired.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact identity values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps failures under their response lifetime.
 */
SourceJsonStatus source_json_schema_unsigned(const SourceJsonValue *, uintmax_t *, SourceJsonIssue *);
#endif
