/** @file
 * @brief Share lossless source-byte/path encoding between captured requests and domain replies.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves exact
 * physical bytes; [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * shares immutable transport values without compiler or project effects.
 */
#ifndef AMA_DELIVERY_SOURCE_WIRE_H
#define AMA_DELIVERY_SOURCE_WIRE_H
#include "source/text/source.h"
#include "tools/output/json/json.h"
/** Encode nullable physical path bytes without replacement/normalization under
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool delivery_source_wire_path(ToolJsonWriter *, const char *);
/** Encode nullable captured {path,content} snapshots, borrowing source/stream
 * owners synchronously under [COMP-04](../../../spec/implementation/compiler.md#comp-04).
 */
bool delivery_source_wire_text(ToolJsonWriter *, const SourceText *);
#endif
