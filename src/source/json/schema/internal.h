/** @file
 * @brief Declare original-source schema failure reporting.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares exact data validation without sharing compiler or mutation owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * lossless bytes and exact integer offsets without permissive conversion;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * complete/failed decoded payloads under the caller's operation arena.
 */
#ifndef AMA_SOURCE_JSON_SCHEMA_INTERNAL_H
#define AMA_SOURCE_JSON_SCHEMA_INTERNAL_H
#include "schema.h"
/** Publish one static explanation at the exact original response value.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_schema_error(const SourceJsonValue *, SourceJsonIssue *, const char *);
#endif
