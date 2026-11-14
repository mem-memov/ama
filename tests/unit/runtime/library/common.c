/** @file
 * @brief Compose address-sensitive native trees with the explicit component service.
 */
#ifndef AMA_LIBRARY_LIFECYCLE_TEST_COMMON
#define AMA_LIBRARY_LIFECYCLE_TEST_COMMON
#include "../contexts/common.c"
#include "runtime/library/internal.h"
#include "runtime/program/calls/call.h"
static AmaLibraryService service;
static const AmaLibraryExport exports[] = {{1,2},{2,1}};
/** Initialize ordinary kernel fixtures and an independent explicit C-boundary service. */
static void setup_library(void) {
    setup_contexts();
    AmaStatus status = ama_library_service_init(&service, sources.allocator, &definition, exports, 2);
    assert(status.code == AMA_OK);
}
/** End the service only after every context was consumed, then verify counted cleanup. */
static void finish_library(void) {
    AmaStatus status = ama_library_service_destroy(&service);
    assert(status.code == AMA_OK);
    finish_contexts();
}
/** Create exactly one context and keep its public value separate from core identities. */
static AmaLibraryContext library_context(void) {
    AmaLibraryContext context = {0};
    AmaLibraryResult result = ama_library_context_create(&service, &context);
    assert(result.code == AMA_LIBRARY_OK && context.owner == (uintptr_t)&service && context.identity != 0);
    return context;
}
/** Publish one explicit root through the configured export, without executing a body. */
static AmaLibraryInstance library_root(AmaLibraryContext context, uint64_t exported) {
    AmaLibraryInstance instance = {0};
    AmaLibraryResult result = ama_library_instance_create(&service, context, exported, &instance);
    assert(result.code == AMA_LIBRARY_OK && instance.context == context.identity && instance.identity != 0);
    return instance;
}
#endif
