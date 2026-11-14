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
    if (strcmp(type->plan->type->module->source->logical_path, ".worker") != 0) { return NULL; }
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
    file("generated/project/helper", "@<\nsource&\n.wrapper\n@!\ncopy\n.wrapper\ncopy\n % source&\n");
    file("generated/project/task", "@<\nexecutor&\n.helper\nowner&\n.wrapper\n@!\nprobe\n.worker\nprobe !\n < |1\n%\nexecutor& !\n < owner&\n");
    file("generated/project/wrapper", "@<\nexecutor&\n.helper\n@>\nchild&\n.task\n@!\nbody\n.task\nchild&\n &body\nbody !\n < executor&\n < &\n%\n");
}
#endif
