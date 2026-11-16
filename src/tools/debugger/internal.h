/** @file
 * @brief Share bounded command and streaming observation operations inside the debugger client.
 * [DEBUG-01](../../../spec/implementation/diagnostics.md#debug-01) maps only exact retained operations to source.
 */
#ifndef AMA_TOOLS_DEBUGGER_INTERNAL_H
#define AMA_TOOLS_DEBUGGER_INTERNAL_H
#include "client.h"
#include "tools/output/json/json.h"
#include "runtime/program/diagnostics/trace.h"
/** Every complete input line has one exact finite interpretation; EOF and IO are separate. */
typedef enum {
    AMA_DEBUG_CLIENT_END, AMA_DEBUG_CLIENT_IO, AMA_DEBUG_CLIENT_INVALID,
    AMA_DEBUG_CLIENT_RUN, AMA_DEBUG_CLIENT_RESUME, AMA_DEBUG_CLIENT_INSPECT,
    AMA_DEBUG_CLIENT_DISPOSE, AMA_DEBUG_CLIENT_QUIT
} AmaDebugClientCommand;
/** Borrowed nested enumeration state; each array owns its separator flag. */
typedef struct {
    ToolJsonWriter *writer;
    AmaDebugSession *session;
    AmaContextReference instance;
    bool first;
    AmaStatus status;
} AmaDebugClientOutput;
/** Consume one complete line without truncation, including an unterminated final line. */
AmaDebugClientCommand ama_debug_client_read(FILE *);
/** Serialize exact status code and optional immutable message bytes. */
bool ama_debug_client_status(ToolJsonWriter *, AmaStatus);
/** Serialize all context and local identity fields; emptiness is not liveness. */
bool ama_debug_client_reference(ToolJsonWriter *, AmaContextReference);
/** Serialize a current occurrence independently of its target. */
bool ama_debug_client_membership(ToolJsonWriter *, uint64_t, AmaMembership);
/** Serialize an exact immutable source row or null. */
bool ama_debug_client_location(ToolJsonWriter *, const AmaProgramLocation *);
/** Emit a separator only after a previous array element. */
bool ama_debug_client_next(AmaDebugClientOutput *);
/** Stream one actual instance and its declared fields, cursors and permitted native view. */
bool ama_debug_client_instance(void *, const AmaDebugInstance *);
/** Stream one declared field, including current list occurrences. */
bool ama_debug_client_field(void *, const AmaDebugFieldValue *);
/** Stream one retained lexical cursor without moving its boundary. */
bool ama_debug_client_cursor(void *, const AmaDebugCursor *);
/** Preview only registered native types; absent readers remain explicitly unavailable. */
bool ama_debug_client_native(AmaDebugClientOutput *, const AmaDebugInstance *);
/** Acquire, stream and release one checked inspection session before returning. */
bool ama_debug_client_inspect(const AmaDebugClientHost *, ToolJsonWriter *, AmaStatus *);
#endif
