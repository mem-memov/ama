/** @file
 * @brief Drive one explicitly owned generated host through commands and checked inspection at rest.
 * [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03) exposes actual
 * retained state; [RT-16](../../../spec/implementation/runtime.md#rt-16) keeps execution under host control.
 */
#ifndef AMA_TOOLS_DEBUGGER_CLIENT_H
#define AMA_TOOLS_DEBUGGER_CLIENT_H
#include <stdio.h>
#include "runtime/debugger/debugger.h"
#include "runtime/metadata/host.h"
#include "runtime/context/execution/execution.h"

/** Borrowed synchronous host authority. begin must acquire a fresh zeroed session
 * using the matching catalog and explicit native readers. Callbacks retain no
 * client borrows. execute returns a valid status with program-lifetime message
 * storage and a context-qualified root, or an empty root on failure. dispose
 * must be idempotent, including an uninitialized host.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [RT-16](../../../spec/implementation/runtime.md#rt-16). */
typedef struct {
    void *context;
    AmaContextOutcome (*execute)(void *, AmaAccessMode);
    AmaStatus (*begin)(void *, AmaDebugSession *);
    AmaStatus (*dispose)(void *);
} AmaDebugClientHost;

/** Read exact LF-separated run/resume/inspect/dispose/quit commands, emitting one
 * JSON line per command. EOF also ends the client. Invalid commands/refusals are
 * reported without execution and make the final result unsuccessful. Input or
 * output failure stops further commands. For valid arguments always dispose before returning, even
 * after failed output; release each inspection session before flushing. Streams
 * and host are borrowed, never closed or retained. No client heap is allocated.
 * Null streams or missing host callbacks reject before any host callback.
 * Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03), [RT-16](../../../spec/implementation/runtime.md#rt-16). */
int ama_debug_client_run(const AmaDebugClientHost *, FILE *, FILE *, FILE *);
#endif
