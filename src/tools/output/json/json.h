/** @file
 * @brief Emit lossless structured tool observations while retaining exact UTF-8 bytes and integer identities.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * requires exact source coordinates and forbids presenting failed output as a usable result.
 */
#ifndef AMA_TOOLS_JSON_H
#define AMA_TOOLS_JSON_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
/** The first encoding/output error is retained; the writer never owns its stream. */
typedef enum { TOOL_JSON_OK, TOOL_JSON_IO_ERROR, TOOL_JSON_INVALID_UTF8, TOOL_JSON_INVALID_VALUE } ToolJsonStatus;
/** Linear synchronous output observation; callbacks borrow it and never close or retain its stream. */
typedef struct { FILE *output; ToolJsonStatus status; } ToolJsonWriter;
/** Start a writer over an already open borrowed stream without output effects. */
void tool_json_init(ToolJsonWriter *, FILE *);
/** Write trusted static JSON grammar fragments; all untrusted values use the typed operations below. */
bool tool_json_raw(ToolJsonWriter *, const char *);
/** Encode an exact UTF-8 byte view, including NUL/control bytes; null is permitted only at zero length. */
bool tool_json_text(ToolJsonWriter *, const unsigned char *, size_t);
/** Encode a borrowed nullable C string; null becomes JSON null. */
bool tool_json_string(ToolJsonWriter *, const char *);
/** Encode the exact unsigned identity as a decimal STRING, preserving values beyond JavaScript's integer precision. */
bool tool_json_unsigned(ToolJsonWriter *, uintmax_t);
/** Encode one JSON boolean without truthy/nonzero coercion in the wire representation. */
bool tool_json_boolean(ToolJsonWriter *, bool);
/** Preserve arbitrary bytes (null only at zero length) as {"utf8":text-or-null,"hex":lowercase-hex}; never replace or normalize invalid text. */
bool tool_json_bytes(ToolJsonWriter *, const unsigned char *, size_t);
/** Flush but never close; false retains the first error even after partial output. */
bool tool_json_finish(ToolJsonWriter *);
#endif
