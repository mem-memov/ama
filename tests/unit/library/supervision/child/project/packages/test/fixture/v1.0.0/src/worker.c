/** @file
 * @brief Retain child state and emit deliberate technical failures without process exit.
 * [EMBED-05](../../../../../../../../../../../spec/implementation/c-library.md#embed-05) preserves
 * a primary child failure; the parent chooses its own supervision policy after C return.
 */
#include "worker-api.h"
#include AMA_WORKER_INTERFACE_HEADER
#include AMA_WORKER_BEGIN_HEADER
#include "../../../../../../../probe.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    *state=(AMA_NATIVE(State)){.resource=malloc(1),.address=state};
    if (state->resource==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }
    ama_nested_probe_v1_event(AMA_NESTED_RESOURCE_CREATE);
    return ama_status_success();
}
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    assert(state->address==state && state->resource!=NULL);
    free(state->resource); state->resource=NULL;
    ama_nested_probe_v1_event(AMA_NESTED_RESOURCE_DESTROY);
    return state->fail_destroy ? ama_status_make(AMA_NATIVE_FAILURE,"Secondary child cleanup failure",sizeof("Secondary child cleanup failure")-1)
        : ama_status_success();
}
/** Mock requests deliberately fail at the first step; ordinary text is copied by counted view. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_WORKER_PARAMETERS *parameters,AMA_WORKER_RESULTS *results,
    const AmaNativeServices *services,AmaStatus *status) {
    (void)services;
    assert(state->address==state && state->resource!=NULL);
    AmaByteView request={0}; *status=AMA_WORKER_TEXT(view)(parameters->AMA_INPUT(0)->payload,&request);
    if (status->code!=AMA_OK) { return; }
    bool fault=request.length==5 && memcmp(request.data,"fault",5)==0;
    state->fail_destroy=request.length==13 && memcmp(request.data,"fault-cleanup",13)==0;
    if (fault || state->fail_destroy) {
        *status=ama_status_make(AMA_NATIVE_FAILURE,"Deliberate child execution failure",sizeof("Deliberate child execution failure")-1); return;
    }
    if (state->calls==SIZE_MAX) { *status=ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); return; }
    state->calls++;
    *status=AMA_WORKER_TEXT(replace)(results->AMA_OUTPUT(0)->payload,request);
    if (status->code==AMA_OK) { results->AMA_OUTPUT(1)->payload->value=state->calls>1; }
}
#include AMA_WORKER_END_HEADER
