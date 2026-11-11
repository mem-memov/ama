#ifndef AMA_TEST_METADATA_COMMON_C
#define AMA_TEST_METADATA_COMMON_C

#include <assert.h>
#include <stdlib.h>
#include "runtime/metadata/instance.h"
#include "runtime/metadata/host.h"
#include "runtime/metadata/source.h"
#include "runtime/metadata/pool.h"
#include "runtime/metadata/runtime.h"

static AmaInstanceMetadata caller;
static AmaInstanceMetadata callee;

static AmaInstanceMetadata make_instance(size_t slot) {
    AmaInstanceMetadata instance = {0};
    instance.identity = (AmaSlotIdentity) {7, slot, 1, true, false};
    AmaStatus status = ama_lifetime_begin(&instance.lifetime, 0, AMA_CONSTRUCT_DEFAULT);
    assert(ama_status_is_success(status));
    status = ama_lifetime_finish(&instance.lifetime, false);
    assert(ama_status_is_success(status));
    instance.service.activity = AMA_INSTANCE_READY;
    return instance;
}

static void setup_test(void) {
    caller = make_instance(0);
    callee = make_instance(1);
}

static void destroy_test(void) {
    caller = (AmaInstanceMetadata) {0};
    callee = (AmaInstanceMetadata) {0};
}

#endif
