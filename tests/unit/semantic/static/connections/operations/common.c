/** @file
 * @brief Retain actual callers and original operation identities for call component tests.
 */
#ifndef AMA_TYPED_OPERATION_TEST_COMMON
#define AMA_TYPED_OPERATION_TEST_COMMON
#include "../common.c"

/** Retain a real caller and finite field annotations before examining operations. */
static const StaticTypedScope *prepare_operations(const StaticResultContext **actual) {
    *actual = publication_context(caller_environment);
    const StaticTypedScope *scope;
    bool success = static_connection_context(&connections, *actual, &scope);
    if (!success) {
        fprintf(stderr, "Typed caller failure: %s at bytes %zu..%zu\n", connections.error.message,
            connections.error.primary.span.start, connections.error.primary.span.end);
    }
    assert(success);
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    return scope;
}

/** Select the original source-ordered operation, never a synthetic call node. */
static const AstNode *operation_at(const StaticTypedScope *scope, size_t ordinal) {
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_CALL && ordinal-- == 0) { return event->node; }
    }
    assert(false); return NULL;
}

#endif
