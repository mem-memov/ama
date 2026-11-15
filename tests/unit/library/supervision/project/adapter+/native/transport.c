/** @file
 * @brief Copy child bytes into declared parent values before child diagnostics/contexts retire.
 * [EMBED-05](../../../../../../../spec/implementation/c-library.md#embed-05) forbids
 * forwarding a child-owned diagnostic pointer as a parent program-lifetime status.
 */
/** Atomically establish the ordinary failure's owned description and then its condition bit. */
static AmaStatus AMA_NATIVE(failure)(AMA_EMBED_RESULTS *results,const uint8_t *bytes,size_t length) {
    return AMA_EMBED_FAILURE(set)(results->AMA_OUTPUT(3),(AmaByteView){bytes,length});
}
/** Preserve the detailed child report while its API result still owns the code dependency. */
static AmaStatus AMA_NATIVE(report)(AMA_EMBED_RESULTS *results,const CHILD(Diagnostic) *diagnostic) {
    static const uint8_t fallback[]="Child computation failed";
    return AMA_NATIVE(failure)(results,diagnostic->message==NULL ? fallback : diagnostic->message,
        diagnostic->message==NULL ? sizeof(fallback)-1 : diagnostic->length);
}
/** Healthy steps clear both failure fields; copying a requested snapshot is independent of its context. */
static AmaStatus AMA_NATIVE(publish)(AMA_EMBED_RESULTS *results,CHILD_TEXT(Value) *response,
    CHILD_CONDITION(Value) *previous,uint32_t outcome) {
    CHILD_TEXT(Borrowed) text={0}; CHILD_CONDITION(Borrowed) condition={0};
    bool valid=AMA_NATIVE(accepted)(CHILD_TEXT(view)(response,&text))
        && AMA_NATIVE(accepted)(CHILD_CONDITION(view)(previous,&condition));
    if (!valid) { return ama_status_make(AMA_NATIVE_FAILURE,"Child snapshot read failed",sizeof("Child snapshot read failed")-1); }
    AmaStatus status=AMA_EMBED_FAILURE(clear)(results->AMA_OUTPUT(3));
    if (status.code==AMA_OK) { status=AMA_EMBED_TEXT(replace)(results->AMA_OUTPUT(0)->payload,(AmaByteView){text.data,text.length}); }
    if (status.code==AMA_OK) {
        results->AMA_OUTPUT(1)->payload->value=condition.value!=0;
        results->AMA_OUTPUT(2)->payload->value=outcome==CHILD_YIELDED;
    }
    return status;
}
