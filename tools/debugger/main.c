/** @file
 * @brief Build a command client around one retained generated project assembly, never a precompiled program binary.
 * [RT-16](../../spec/implementation/runtime.md#rt-16) exposes explicit entry/resume/disposal;
 * [DEBUG-03](../../spec/implementation/diagnostics.md#debug-03) permits inspection only of actual retained state.
 */
#include "project.h"
#include "project.c"
#include "tools/debugger/client.h"
#include <string.h>
/** The generated boundary alone prepares and enters the project; no Ama activation survives this call. */
static AmaContextOutcome execute(void *context, AmaAccessMode mode) {
    AmaProjectHost *host = context;
    AmaHostResult result = ama_project_execute(host, mode);
    AmaContextReference root = ama_handle_is_empty(result.root) ? (AmaContextReference){0}
        : (AmaContextReference){host->identity, result.root};
    return (AmaContextOutcome){result.outcome, root, result.status};
}
/** This default client grants no native read adapter and refuses an uninitialized or disposed host. */
static AmaStatus begin(void *context, AmaDebugSession *session) {
    AmaProjectHost *host = context;
    if (!host->initialized || host->disposed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    return ama_debug_begin(&host->registry, host->identity, &ama_project_debug_catalog, NULL, 0, session);
}
/** Generated disposal is idempotent, including EOF before first execution. */
static AmaStatus dispose(void *context) { return ama_project_dispose(context); }
/** A separately compiled executable owns this generated host until the command loop returns. */
int main(int argc, char **argv) {
    if (argc != 3 || strcmp(argv[1], "--responses") != 0 || argv[2][0] == 0) {
        (void)fputs("Usage: ama-debugger --responses NEW_FILE\n", stderr);
        return EXIT_FAILURE;
    }
    FILE *responses = fopen(argv[2], "wbx");
    if (responses == NULL) {
        (void)fputs("Cannot exclusively create debugger response file\n", stderr);
        return EXIT_FAILURE;
    }
    AmaProjectHost host = {0};
    AmaDebugClientHost client = {&host, execute, begin, dispose};
    int result = ama_debug_client_run(&client, stdin, responses, stderr);
    if (fclose(responses) != 0) {
        (void)fputs("Cannot close debugger response file\n", stderr);
        return EXIT_FAILURE;
    }
    return result;
}
