/** @file
 * @brief Own decoded protocol names, physical byte paths and coherent captured source data.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares data owners without compiler or project effects;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves exact bytes;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires coherent repeated physical captures;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates retained
 * values from temporary syntax/byte-decoding storage.
 */
#ifndef AMA_SOURCE_JSON_CAPTURE_INTERNAL_H
#define AMA_SOURCE_JSON_CAPTURE_INTERNAL_H
#include "capture.h"
#include "../schema/schema.h"
/** Exact physical key/content index under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
struct SourceJsonCapturedText { SourceJsonCapturedText *next; const SourceText *source; };
/** Preserve original field location/static prose under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
SourceJsonStatus source_json_capture_error(const SourceJsonValue *, SourceJsonIssue *, const char *);
/** Retain storage failure without publishing a result under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
SourceJsonStatus source_json_capture_memory(const SourceJsonValue *, SourceJsonIssue *);
#endif
