/** @file
 * @brief Retire parsed source-request metadata independently of borrowed argv and source files.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains failed prefixes without performing hidden source cleanup or effects.
 */
#include "request.h"
#include <string.h>
void project_source_request_init(ProjectSourceRequest *request, SourceAllocator allocator) {
    memset(request, 0, sizeof(*request)); source_arena_init(&request->arena, allocator);
}
void project_source_request_destroy(ProjectSourceRequest *request) {
    source_arena_destroy(&request->arena); memset(request, 0, sizeof(*request));
}
