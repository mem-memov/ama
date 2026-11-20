/** @file
 * @brief Observe native cloning and create an available owner whose owned task can independently become busy.
 */
#ifndef AMA_SNAPSHOT_TEST_COMMON
#define AMA_SNAPSHOT_TEST_COMMON
#include "../execution/common.c"

/** Clone markers expose any resource preparation before complete source
 * preflight. Optional cleanup markers expose the unmodified source payload.
 */
static const char *observe_snapshot_worker(const ScaffoldType *type, AbiOperation operation) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/worker") != 0) { return NULL; }
    if (operation == ABI_CLONE) {
        return " { assert(source->address==source); if(fputs(\"cloneworker\\n\",stdout)==EOF) return ama_status_make(AMA_IO_FAILURE,NULL,0); *destination=*source; destination->address=destination; return ama_status_success(); }\n";
    }
    if (operation == ABI_DESTROY) {
        return " { assert(state->address==state); return printf(\"dropworker=%zu\\n\",state->calls)<0 ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
    }
    return NULL;
}

/** Default destruction stays silent for the whole-source availability pair. */
static const char *observe_snapshot_clone(const ScaffoldType *type, AbiOperation operation) {
    return operation == ABI_CLONE ? observe_snapshot_worker(type, operation) : NULL;
}

/** Reference edges break eager cycles. The helper owns a distinct destination;
 * it can snapshot an available wrapper without being inside that ownership tree.
 */
static void prepare_availability_snapshot(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/helper", "@<\nsource&\n/test/fixture/v1.0.0/src/wrapper\n@!\ncopy\n/test/fixture/v1.0.0/src/wrapper\ncopy\n % source&\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@<\nexecutor&\n/test/fixture/v1.0.0/src/helper\nowner&\n/test/fixture/v1.0.0/src/wrapper\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\nexecutor& !\n < owner&\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/wrapper", "@<\nexecutor&\n/test/fixture/v1.0.0/src/helper\n@>\nchild&\n/test/fixture/v1.0.0/src/task\n@!\nbody\n/test/fixture/v1.0.0/src/task\nchild&\n &body\nbody !\n < executor&\n < &\n%\n");
}
#endif
