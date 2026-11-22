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
#ifndef AMA_SOURCE_JSON_CAPTURE_H
#define AMA_SOURCE_JSON_CAPTURE_H
#include "../json.h"
/** Private exact-path coherence index under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef struct SourceJsonCapturedText SourceJsonCapturedText;
/** Retained names/index nodes belong to arena; source/path/byte snapshots belong
 * to sources. No I/O, role inference, Ama parsing or compiler state is owned.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps every
 * complete/failed allocation destructible until operation teardown.
 */
typedef struct { SourceArena arena; SourceManager sources; SourceJsonCapturedText *texts; } SourceJsonCapture;
/** Initialize empty result ownership under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_json_capture_init(SourceJsonCapture *, SourceAllocator);
/** Retire names/indexes before their captured source records under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void source_json_capture_destroy(SourceJsonCapture *);
/** Copy a nonempty NUL-free decoded string; syntax storage may expire after
 * return. Output changes only on success under
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
SourceJsonStatus source_json_capture_string(SourceJsonCapture *, const SourceJsonValue *, const char **, SourceJsonIssue *);
/** Decode nullable physical byte paths, refusing empty or NUL-truncated values.
 * scratch is an independently owned arena, distinct from capture->arena. Names
 * survive scratch/syntax teardown; failure preserves output and retained captures.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids repair.
 */
SourceJsonStatus source_json_capture_path(SourceJsonCapture *, SourceArena *, const SourceJsonValue *, const char **, SourceJsonIssue *);
/** Decode nullable {path,content} source blobs without validating a source role.
 * Binary opaque ABI data remains binary until actual native classification.
 * Equal physical keys must have equal bytes and reuse one stable SourceText;
 * conflicting data is refused without replacing prior captures or caller output.
 * Scratch is distinct from result storage; successful data survives syntax and
 * original JSON release. Failure diagnostics borrow that original JSON source.
 * Work includes exact retained-path/byte comparisons; memory retains names/index
 * nodes and unique source snapshots plus failed scratch until destruction.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires coherent capture, while [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * fixes independent scratch/result lifetimes.
 */
SourceJsonStatus source_json_capture_text(SourceJsonCapture *, SourceArena *, const SourceJsonValue *, const SourceText **, SourceJsonIssue *);
#endif
