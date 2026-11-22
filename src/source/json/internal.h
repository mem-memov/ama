/** @file
 * @brief Define JSON cursor and explicit container-continuation contracts.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * shares immutable protocol syntax without sharing project mutation or compiler owners;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires
 * exact bytes and source coordinates rather than lossy scalar conversions;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains
 * partial trees and decoded strings under one explicit operation arena.
 */
#ifndef AMA_SOURCE_JSON_INTERNAL_H
#define AMA_SOURCE_JSON_INTERNAL_H
#include "json.h"
/** Object/array grammar states distinguish an empty container from a forbidden
 * trailing comma; no delimiter or key is inferred from nearby source text.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef enum { JSON_FIRST_KEY, JSON_KEY, JSON_COLON, JSON_VALUE,
    JSON_FIRST_VALUE, JSON_SEPARATOR } SourceJsonState;
/** One retained explicit container continuation; last permits O(1) child append.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef struct SourceJsonFrame {
    struct SourceJsonFrame *parent;
    SourceJsonValue *value;
    SourceJsonValue *last;
    SourceJsonText key;
    SourceJsonState state;
} SourceJsonFrame;
/** Synchronous cursor with immutable source bytes and first-error retention.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef struct {
    SourceArena *arena;
    const SourceText *source;
    const unsigned char *bytes;
    size_t length, offset;
    SourceJsonFrame *frame;
    SourceJsonValue *root;
    SourceJsonStatus status;
    SourceJsonIssue issue;
} SourceJsonCursor;
/** Retain the first original-byte diagnostic, returning false for propagation.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
bool source_json_fail(SourceJsonCursor *, SourceJsonStatus, const char *);
/** Consume only the four JSON whitespace bytes, independently of Ama layout.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
void source_json_space(SourceJsonCursor *);
/** Decode one quoted token in two passes, publishing exact-sized storage only
 * after valid escapes/surrogate pairing; failure never publishes a token.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
bool source_json_string(SourceJsonCursor *, SourceJsonText *);
/** Consume exactly one JSON numeric spelling, retaining its borrowed bytes.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
bool source_json_number(SourceJsonCursor *, SourceJsonText *);
/** Read one scalar or container opening, attach it privately and push explicit
 * container state when needed. Root publication remains the parser's last step.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
bool source_json_value(SourceJsonCursor *);
/** Perform one object/array grammar transition, including exact key uniqueness.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
bool source_json_container(SourceJsonCursor *);
#endif
