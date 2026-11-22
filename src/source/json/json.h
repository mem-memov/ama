/** @file
 * @brief Declare exact response JSON trees and their arena/source lifetimes.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * shares immutable protocol syntax without sharing project mutation or compiler owners;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires
 * exact bytes and source coordinates rather than lossy scalar conversions;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains
 * partial trees and decoded strings under one explicit operation arena.
 */
#ifndef AMA_SOURCE_JSON_H
#define AMA_SOURCE_JSON_H
#include "source/memory/arena.h"
#include "source/text/source.h"
/** Full JSON scalar/container categories; numeric text is never converted.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef enum { SOURCE_JSON_OBJECT, SOURCE_JSON_ARRAY, SOURCE_JSON_STRING,
    SOURCE_JSON_NUMBER, SOURCE_JSON_BOOLEAN, SOURCE_JSON_NULL } SourceJsonKind;
/** Length-delimited decoded string or borrowed numeric spelling. Embedded NUL
 * is retained. Decoded strings additionally have a trailing zero byte; numeric
 * spellings remain borrowed bounded views. origin names the complete original token including string quotes.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef struct { const unsigned char *bytes; size_t length; SourceSpan origin; } SourceJsonText;
/** Arena-owned tree node. next/first link siblings/children in source order.
 * An object child carries its exact decoded key; array/root keys are absent.
 * Numbers borrow original response bytes, decoded strings belong to the arena.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef struct SourceJsonValue {
    struct SourceJsonValue *next;
    struct SourceJsonValue *first;
    SourceJsonKind kind;
    SourceSpan origin;
    SourceJsonText key;
    SourceJsonText text;
    bool boolean;
} SourceJsonValue;
/** Syntax/UTF-8 and allocation failures remain distinct, with original location.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef enum { SOURCE_JSON_OK, SOURCE_JSON_INVALID, SOURCE_JSON_OUT_OF_MEMORY } SourceJsonStatus;
/** Borrow original response source and static prose until source teardown.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
typedef struct { SourceSpan origin; const char *message; } SourceJsonIssue;
/** Parse one complete UTF-8 JSON document, rejecting duplicate decoded object
 * keys. Use an explicit container stack, with no recursion or arbitrary depth
 * limit. Preserve number spelling and string bytes (including escaped NUL).
 * Caller retains source/allocator through result/diagnostic consumption. All
 * complete/failed allocations stay arena-owned; output changes only on success.
 * No I/O, process calls, type inference or project mutation occurs. Syntax success
 * does not certify the domain schema or authorize source operations.
 * Work is O(input bytes + sum of per-object sibling key comparisons including
 * compared key bytes); decoded-key uniqueness deliberately uses exact comparison.
 * Space is O(nodes + containers + decoded string/key bytes).
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
SourceJsonStatus source_json_parse(SourceArena *, const SourceText *,
    const SourceJsonValue **, SourceJsonIssue *);
#endif
