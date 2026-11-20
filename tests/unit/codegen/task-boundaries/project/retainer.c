/** @file
 * @brief Retain native-only callback records across context-wide failure and acknowledge them from the host afterward.
 * [ASYNC-11](../../../../../spec/language/async-and-parallelism.md#async-11)
 * requires wait detachment before failed returns; native endpoints and owned bytes
 * contain no Ama handle, view or payload address.
 */
#include "ama_n_7198aa6bea571660cde5ac49c780ac37700437ff596d16920c53918ff606474e.interface.h"
#include "ama_n_7198aa6bea571660cde5ac49c780ac37700437ff596d16920c53918ff606474e.begin.h"
#include "native/events/assembly.h"
#include <assert.h>
#include <stdio.h>
/** The test host owns these two native callback credits independently of Ama lifetime. */
static AmaNativeEventSubmission AMA_NATIVE(endpoints)[2];
/** Each callback owns counted binary bytes; no borrowed Ama data crosses failure cleanup. */
static AmaNativeEventBuffer AMA_NATIVE(buffers)[2];
/** A refused resume of a failed context must not execute this native entry again. */
static size_t AMA_NATIVE(captures);
/** Default payload construction acquires no callback credit. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Context destruction cannot consume independently host-owned callback rights. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Capture one native endpoint while its temporary checked view is live. */
static AmaStatus AMA_NATIVE(capture)(const ama_t_3211b272d4ffc58b35668036530071d3838391fee0aa5c4208851404f0010b09 *operation,
    void *resource) {
    return ama_native_event_submission_clone(&operation->payload->submission, resource);
}
/** Retain two endpoints and prepare bytes before failure; publication is deliberately delayed until the host observes failed. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_4b6376af42f3de26bcdc9c0f027da71f0f02dd73fd4046304fe106ded375abc9 *parameters,
    ama_t_956f7576a1f00c5b67f9515792a3193a08f5dea6f44ba8e59a8dde29b75f1257 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)results; (void)services;
    AMA_NATIVE(captures)++;
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage, AMA_NATIVE(capture), &AMA_NATIVE(endpoints)[0]);
    if (status->code != AMA_OK) { return; }
    *status = parameters->AMA_INPUT(1)->with_read(parameters->AMA_INPUT(1)->storage, AMA_NATIVE(capture), &AMA_NATIVE(endpoints)[1]);
    if (status->code != AMA_OK) { return; }
    const unsigned char bytes[] = {0, 255};
    for (size_t index = 0; index < 2; index++) {
        *status = ama_native_event_buffer_copy(&AMA_NATIVE(endpoints)[index], (AmaByteView){bytes, sizeof(bytes)}, &AMA_NATIVE(buffers)[index]);
        if (status->code != AMA_OK) { return; }
    }
}
/** Assert cleanup already closed the source, then consume every host-owned callback/byte credit exactly once. */
static void AMA_NATIVE(acknowledge)(void) {
    assert(AMA_NATIVE(captures) == 1);
    for (size_t index = 0; index < 2; index++) {
        assert(AMA_NATIVE(endpoints)[index].state != NULL && AMA_NATIVE(buffers)[index].length == 2);
        AmaNativeEventPostResult posted = ama_native_event_post(&AMA_NATIVE(endpoints)[index], 7, &AMA_NATIVE(buffers)[index]);
        assert(posted == AMA_EVENT_CLOSED);
        assert(AMA_NATIVE(buffers)[index].owner != NULL && AMA_NATIVE(buffers)[index].data[1] == 255);
        ama_native_event_buffer_drop(&AMA_NATIVE(buffers)[index]);
        ama_native_event_submission_drop(&AMA_NATIVE(endpoints)[index]);
        assert(AMA_NATIVE(endpoints)[index].state == NULL && AMA_NATIVE(buffers)[index].owner == NULL);
    }
    puts("callbacks=closed,closed");
}
#include "ama_n_7198aa6bea571660cde5ac49c780ac37700437ff596d16920c53918ff606474e.end.h"
