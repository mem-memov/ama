/** @file
 * @brief Publish immutable schemas and worker results outside runtime metadata gates.
 * Joins publish observations; workers share only registry/schema/native synchronization.
 */
#ifndef AMA_ISOLATION_TEST_COMMON_C
#define AMA_ISOLATION_TEST_COMMON_C
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include "runtime/context/context.h"
#include "support/barrier.c"
#include "support/payload.c"

static const unsigned char literal_bytes[] = {37};
static const AmaProgramSource literal = {
    .kind = AMA_SOURCE_LITERAL, .literal = {literal_bytes, 1}, .type_id = 1
};
static const AmaContextConstant constant = {
    .type_id = 1, .initializers = &literal, .initializer_count = 1
};
static const AmaContextType type = {
    .type_id = 1, .size = sizeof(IsolatedPayload), .alignment = _Alignof(IsolatedPayload),
    .slots_per_block = 1, .generation_limit = 3,
    .hooks = {payload_initialize, payload_literal, payload_clone, payload_destroy}
};
static const AmaContextDefinition definition = {
    .types = &type, .type_count = 1, .constants = &constant, .constant_count = 1
};

/** Each worker owns its output until the main thread joins. */
typedef struct { AmaContextRegistry *registry; uint64_t identity; } FactoryWorker;

/** Construct actual constants while the other factory is inside its native hook. */
static void *factory_worker(void *argument) {
    FactoryWorker *worker = argument;
    AmaStatus status = ama_context_create(worker->registry, &definition, &worker->identity);
    assert(status.code == AMA_OK && worker->identity != 0);
    return NULL;
}

/** Clean actual constants outside the registry membership gate. */
static void *disposal_worker(void *argument) {
    FactoryWorker *worker = argument;
    AmaStatus status = ama_context_dispose(worker->registry, worker->identity);
    assert(status.code == AMA_OK);
    return NULL;
}

/** Start both workers before joining; overlap uses no elapsed-time oracle. */
static void run_pair(FactoryWorker workers[2], void *(*operation)(void *)) {
    pthread_t threads[2];
    for (size_t index = 0; index < 2; index++) {
        int result = pthread_create(&threads[index], NULL, operation, &workers[index]);
        assert(result == 0);
    }
    for (size_t index = 0; index < 2; index++) {
        void *value = workers;
        int result = pthread_join(threads[index], &value);
        assert(result == 0 && value == NULL);
    }
}

/** Borrow native state only under its origin context lease. */
static IsolatedPayload *resolve_payload(const AmaContextLease *lease, AmaContextReference reference) {
    AmaObjectType *object = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_context_resolve(lease, reference, AMA_ACCESS_READ, &object, &cell);
    assert(status.code == AMA_OK);
    return ama_pool_payload(&object->pool, cell);
}
#endif
