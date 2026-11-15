/** @file
 * @brief Publish retained native data through declared typed Ama results.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * permits only the executor to import native completions into Ama state.
 */
#include <stdbool.h>
struct AMA_NATIVE(State) { bool present; };
#include "native/events/event.h"
/** Exact generated exclusive view; no pointer may escape its native activation. */
typedef struct ama_t_5cd130a48cf871c6976b8d28b7d3db09d549929b4b6ff9649339e81ffadf642e ama_t_5cd130a48cf871c6976b8d28b7d3db09d549929b4b6ff9649339e81ffadf642e;
/** Prepare both owned values before replacing packet contents, then publish presence last. */
AmaStatus AMA_NATIVE(publish)(ama_t_5cd130a48cf871c6976b8d28b7d3db09d549929b4b6ff9649339e81ffadf642e *, const AmaNativeEventCompletion *);
