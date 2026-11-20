/** @file
 * @brief Copy independent child snapshots into declared parent results.
 * [EMBED-07](../../../../../../../../../../../../spec/implementation/c-library.md#embed-07) and
 * [EMBED-08](../../../../../../../../../../../../spec/implementation/c-library.md#embed-08) require
 * exclusive foreign ownership and normal C returns at every step boundary.
 */
/** Copy wire data before releasing the independent child owners, preserving the first failure. */
static AmaStatus AMA_NATIVE(publish)(CHILD_TEXT(Value) *response, CHILD_CONDITION(Value) *previous,
    AMA_EMBED_RESULTS *results, uint32_t outcome) {
    CHILD_TEXT(Borrowed) text={0};
    CHILD_CONDITION(Borrowed) condition={0};
    bool valid=AMA_NATIVE(accepted)(CHILD_TEXT(view)(response,&text))
        && AMA_NATIVE(accepted)(CHILD_CONDITION(view)(previous,&condition));
    AmaStatus status=valid ? AMA_EMBED_TEXT(replace)(results->AMA_OUTPUT(0)->payload,
        (AmaByteView){text.data,text.length})
        : ama_status_make(AMA_NATIVE_FAILURE,"Child snapshot read failed",sizeof("Child snapshot read failed")-1);
    if (status.code==AMA_OK) {
        results->AMA_OUTPUT(1)->payload->value=condition.value!=0;
        results->AMA_OUTPUT(2)->payload->value=outcome==CHILD_YIELDED;
    }
    return status;
}
