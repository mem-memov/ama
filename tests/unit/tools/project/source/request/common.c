/** @file
 * @brief Share counted source-request ownership and exact raw argument observations.
 * @see [T277 entry review](../../../../../../docs/implementation/source-operations.md#t277-entry-review-after-accepted-t276)
 */
#ifndef AMA_PROJECT_SOURCE_REQUEST_TEST_COMMON
#define AMA_PROJECT_SOURCE_REQUEST_TEST_COMMON
#include "../../../../source/common.c"
#include "../../../../../../tools/project/command/source/internal.h"
/** Every case uses a real fresh request owner; refusal never fabricates ready data. */
static void expect_source_request(int count, const char *const *arguments, ProjectSourceRequestStatus expected) {
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    assert(project_source_request_parse(&request, count, arguments) == expected);
    assert(request.valid == (expected == PROJECT_SOURCE_REQUEST_OK));
    project_source_request_destroy(&request); assert(!live_allocations);
}
#endif
