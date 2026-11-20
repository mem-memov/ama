/** @file
 * @brief Apply explicit restart/supervision policy without continuing a failed child context.
 * [EMBED-05](../../../../../../../../../../../../spec/implementation/c-library.md#embed-05) permits
 * a healthy supervisor to return declared failure data. Own construction/report failures
 * remain technical; no heuristic interprets message text as an error category.
 */
/** Native completion never yields the parent implicitly; the outer Ama body owns that decision. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_EMBED_PARAMETERS *parameters,AMA_EMBED_RESULTS *results,
    const AmaNativeServices *services,AmaStatus *status) {
    (void)services;
    struct AMA_NATIVE(Child) *child=state->child;
    if (child==NULL) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }
    if (parameters->AMA_INPUT(1)->payload->value!=0) {
        *status=AMA_NATIVE(close_child)(child);
        if (status->code==AMA_OK) { *status=AMA_NATIVE(open_child)(child); }
        if (status->code!=AMA_OK) { return; }
    }
    if (child->outcome==CHILD_FAILED) {
        static const uint8_t terminal[]="Child failed; explicit reset required";
        *status=parameters->AMA_INPUT(2)->payload->value!=0
            ? ama_status_make(AMA_NATIVE_FAILURE,"Child already failed",sizeof("Child already failed")-1)
            : AMA_NATIVE(failure)(results,terminal,sizeof(terminal)-1);
        return;
    }
    AmaByteView message={0}; *status=AMA_EMBED_TEXT(view)(parameters->AMA_INPUT(0)->payload,&message);
    if (status->code!=AMA_OK) { return; }
    CHILD_TEXT(Borrowed) input={message.data,message.length};
    CHILD_TEXT(Value) *response=NULL; CHILD_CONDITION(Value) *previous=NULL;
    CHILD(ApiResult) result=AMA_NATIVE(call_child)(child,&input,&response,&previous);
    if (result.code!=0) {
        *status=ama_status_make(AMA_NATIVE_FAILURE,"Child call admission rejected",sizeof("Child call admission rejected")-1);
    } else if (child->outcome==CHILD_FAILED) {
        if (parameters->AMA_INPUT(2)->payload->value!=0) {
            *status=ama_status_make(AMA_NATIVE_FAILURE,"Child execution failed",sizeof("Child execution failed")-1);
        } else {
            *status=AMA_NATIVE(report)(results,&result.diagnostic);
        }
    } else {
        *status=AMA_NATIVE(publish)(results,response,previous,child->outcome);
    }
    bool result_released=CHILD(result_release)(&result)==0;
    bool text_released=AMA_NATIVE(accepted)(CHILD_TEXT(release)(&response));
    bool condition_released=AMA_NATIVE(accepted)(CHILD_CONDITION(release)(&previous));
    if (status->code==AMA_OK && (!result_released || !text_released || !condition_released)) {
        *status=ama_status_make(AMA_NATIVE_FAILURE,"Child API owner release failed",sizeof("Child API owner release failed")-1);
    }
    if (child->outcome==CHILD_FAILED) {
        /* A failed context reports its primary status again during admitted disposal.
         * Consumption, not that numeric primary code, proves resource retirement. */
        (void)AMA_NATIVE(close_child)(child);
        if (child->context.opaque[0]!=0 && status->code==AMA_OK) {
            *status=ama_status_make(AMA_NATIVE_FAILURE,"Child context remained live",sizeof("Child context remained live")-1);
        }
    }
}
