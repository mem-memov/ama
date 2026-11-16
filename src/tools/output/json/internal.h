/** @file
 * @brief Keep checked byte writes private to structured tool output encoders.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves the first output failure and prevents later writes from masquerading as completion.
 */
#ifndef AMA_TOOLS_JSON_INTERNAL_H
#define AMA_TOOLS_JSON_INTERNAL_H
#include "json.h"
/** Append exact bytes without ownership transfer; zero length has no stream effect. */
bool tool_json_write(ToolJsonWriter *, const void *, size_t);
#endif
