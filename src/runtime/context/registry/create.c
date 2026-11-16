/**
 * @file
 * @brief Pin an explicit registry while constructing one unpublished independent context.
 *
 * [RT-01](../../../../spec/implementation/runtime.md#rt-01) owns constants per context.
 * [EMBED-02](../../../../spec/implementation/c-library.md#embed-02) requires factory rollback
 * without a partially published handle. This internal factory publishes a context
 * identity; checked public export transports retain their separate library boundary.
 * [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02) reports
 * accepted creation failures before destroying their diagnostic context, including
 * allocation-free failure before the context record itself exists.
 */
#include <string.h>
#include "../internal.h"

/** Admit one private lifecycle operation before allocation/hooks. pending pins registry C
 * lifetime while metadata is unlocked, allowing callbacks and independent context operations.
 * Component contract: [RT-01](../../../../spec/implementation/runtime.md#rt-01), [EMBED-02](../../../../spec/implementation/c-library.md#embed-02), [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02). */
static AmaStatus context_factory_begin(AmaContextRegistry *registry) {
    AmaStatus status = ama_context_registry_lock(registry);
    if (status.code != AMA_OK) { return status; }
    if (registry->closed) { status = ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    else if (registry->pending == SIZE_MAX) { status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    else { registry->pending++; }
    ama_context_registry_unlock(registry);
    return status;
}

/** End a pinned factory and optionally publish one completely constructed record. The pending
 * credit proves close/destroy cannot invalidate this owner before membership publication.
 * Component contract: [RT-01](../../../../spec/implementation/runtime.md#rt-01), [EMBED-02](../../../../spec/implementation/c-library.md#embed-02), [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02). */
static void context_factory_end(AmaContextRegistry *registry, AmaContext *context) {
    (void)ama_context_registry_lock(registry);
    if (context != NULL) {
        context->next = registry->first;
        registry->first = context;
    }
    registry->pending--;
    ama_context_registry_unlock(registry);
}

/** Report one accepted factory failure while its source metadata remains alive.
 * Before the record exists, borrow immutable definition locations through a
 * stack-only diagnostic view; no allocation or implicit Ama root is needed.
 * Component contract: [RT-01](../../../../spec/implementation/runtime.md#rt-01), [EMBED-02](../../../../spec/implementation/c-library.md#embed-02), [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02). */
static void context_factory_report(AmaContext *context, const AmaContextDefinition *definition, AmaStatus status) {
    if (context != NULL) {
        if (context->status.code == AMA_OK) { context->world.report(context->world.report_context, status); }
    } else if (definition->report != NULL) { definition->report(definition->report_context, status); }
    else {
        AmaWorld world = {0};
        AmaProgram program = {.world = &world, .locations = definition->locations,
            .location_count = definition->location_count,
            .initialization_operation = definition->initialization_operation};
        ama_program_report(&program, status);
    }
}

/** Construct private descriptors/constants and publish an opaque number only after success.
 * Failed attempts may consume identity space, never recycle it. Cleanup runs without metadata
 * locks and never executes an Ama body or creates an implicit root instance.
 * Component contract: [RT-01](../../../../spec/implementation/runtime.md#rt-01), [EMBED-02](../../../../spec/implementation/c-library.md#embed-02), [DEBUG-02](../../../../spec/implementation/diagnostics.md#debug-02). */
AmaStatus ama_context_create(AmaContextRegistry *registry, const AmaContextDefinition *definition,
    uint64_t *identity) {
    if (identity == NULL || *identity != 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaStatus status = ama_context_validate(definition);
    if (status.code != AMA_OK) { return status; }
    status = context_factory_begin(registry);
    if (status.code != AMA_OK) { return status; }
    uint64_t issued = 0;
    status = ama_context_identity_issue(&issued);
    AmaContext *context = NULL;
    if (status.code == AMA_OK) {
        context = registry->allocator.allocate(registry->allocator.context, sizeof(*context));
        if (context == NULL) { status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        else {
            memset(context, 0, sizeof(*context));
            context->identity = issued;
            status = ama_context_prepare(context, definition, registry->allocator);
        }
    }
    if (status.code != AMA_OK) { context_factory_report(context, definition, status); }
    if (status.code != AMA_OK && context != NULL) {
        status = ama_context_cleanup(context, status);
        registry->allocator.release(registry->allocator.context, context);
        context = NULL;
    }
    context_factory_end(registry, context);
    if (status.code == AMA_OK) { *identity = issued; }
    return status;
}
