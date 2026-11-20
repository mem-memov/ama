/** @file
 * @brief Finish each child C activation before returning from the parent native entry.
 * [EMBED-07](../../../../../../../../../../../../spec/implementation/c-library.md#embed-07) and
 * [EMBED-08](../../../../../../../../../../../../spec/implementation/c-library.md#embed-08) require
 * exclusive foreign ownership and normal C returns at every step boundary.
 */
/** Resume consumes the child's earlier input; a fresh parent input is used only for child entry. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const AMA_EMBED_PARAMETERS *parameters, AMA_EMBED_RESULTS *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)services;
    struct AMA_NATIVE(Child) *child=state->child;
    if (child==NULL) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }
    CHILD_TEXT(Value) *response=NULL;
    CHILD_CONDITION(Value) *previous=NULL;
    AmaByteView message={0};
    *status=AMA_EMBED_TEXT(view)(parameters->AMA_INPUT(0)->payload,&message);
    if (status->code!=AMA_OK) { return; }
    CHILD_TEXT(Borrowed) input={message.data,message.length};
    AMA_EMBED_BEFORE_CHILD(child->context,child->instance);
    CHILD(ApiResult) result=child->outcome==CHILD_YIELDED
        ? CHILD_EXPORT(resume)(child->context,child->instance,&response,&previous,&child->outcome)
        : CHILD_EXPORT(entry)(child->context,child->instance,&input,&response,&previous,&child->outcome);
    bool accepted=AMA_NATIVE(accepted)(result);
    if (!accepted || child->outcome==CHILD_FAILED) {
        *status=ama_status_make(AMA_NATIVE_FAILURE,"Child execution failed",sizeof("Child execution failed")-1);
    } else {
        *status=AMA_NATIVE(publish)(response,previous,results,child->outcome);
    }
    bool text_released=AMA_NATIVE(accepted)(CHILD_TEXT(release)(&response));
    bool condition_released=AMA_NATIVE(accepted)(CHILD_CONDITION(release)(&previous));
    if (status->code==AMA_OK && (!text_released || !condition_released)) {
        *status=ama_status_make(AMA_NATIVE_FAILURE,"Child snapshot release failed",sizeof("Child snapshot release failed")-1);
    }
}
