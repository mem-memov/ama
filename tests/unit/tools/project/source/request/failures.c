/** @file
 * @brief Exhaust parsed-root/exclusion allocation prefixes and checked representation boundaries.
 * @see [T277 entry review](../../../../../../docs/implementation/source-operations.md#t277-entry-review-after-accepted-t276)
 */
#include <stdint.h>
/** Accepted request ownership remains unchanged on repeat; each failed actual
 * allocation frees every prior block and never claims a valid partial request. */
static void it_retires_every_source_request_allocation_and_preserves_repeat_results(void) {
    setup_test();
    const char *arguments[] = {"ama-project", "move", "source", "destination", "--apply", "--extend-package", "--replace",
        "--root", "project", "--exclude", "project", "build", "--standard-root", "standard",
        "--exclude", "standard", "excluded", "--destination-root", "target", "--destination-exclude", "target", "cache"};
    size_t count = sizeof(arguments)/sizeof(*arguments), total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        ProjectSourceRequestStatus status = project_source_request_parse(&request, (int)count, arguments);
        if (!failure) {
            assert(status == PROJECT_SOURCE_REQUEST_OK && request.valid); total = allocation_count - before; assert(total);
            const ProjectSourceRootRequest *roots = request.roots; const char *compiler = request.compiler;
            size_t calls = allocation_count;
            assert(project_source_request_parse(&request, 0, NULL) == PROJECT_SOURCE_REQUEST_INVALID
                && request.valid && request.roots == roots && request.compiler == compiler && allocation_count == calls);
        } else { assert(status == PROJECT_SOURCE_REQUEST_MEMORY && !request.valid && request.attempted); }
        failure_at = 0; project_source_request_destroy(&request); assert(!live_allocations);
    }
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    ProjectSourceRequestWork work = {.request = &request}; size_t before = allocation_count;
    assert(project_source_request_reserve(&work, SIZE_MAX) == PROJECT_SOURCE_REQUEST_LIMIT && allocation_count == before);
    assert(project_source_request_reserve(&work, 0) == PROJECT_SOURCE_REQUEST_OK && allocation_count == before);
    assert(!project_source_option(NULL) && !project_source_option("--preview=1"));
    project_source_request_destroy(&request);
    assert(project_source_request_parse(NULL, (int)count, arguments) == PROJECT_SOURCE_REQUEST_INVALID);
    destroy_test();
}
