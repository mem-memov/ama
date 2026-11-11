#ifndef AMA_TEST_LIFECYCLE_COMMON_C
#define AMA_TEST_LIFECYCLE_COMMON_C

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "model/project/payload.h"
#include "runtime/lifecycle/errors.h"

typedef struct {
    AmaLifetime lifetime;
    Payload native;
    Payload children[3];
    size_t destroyed[4];
    size_t destruction_count;
    AmaStatus diagnostics[4];
    size_t diagnostic_count;
} Model;

typedef struct {
    size_t children;
    AmaConstructorKind constructor;
    size_t failure_ordinal;
    unsigned cleanup_failures;
} Scenario;

#include "support/cleanup.c"
#include "support/construction.c"

static void expect_finalized(const Model *model) {
    assert(model->lifetime.phase == AMA_LIFETIME_FINALIZED);
    assert(!model->lifetime.cleanup_active && !model->lifetime.native_live);
    assert(model->lifetime.live_children == 0 && model->native.resource == NULL);
    for (size_t index = 0; index < 3; index++) {
        assert(model->children[index].resource == NULL);
    }
}

#endif
