/** @file
 * @brief Exercise an actual parent C helper frame around a complete foreign host call.
 * [EMBED-08](../../../../../../../spec/implementation/c-library.md#embed-08) requires
 * normal child return before the helper can finish and the outer Ama program can yield.
 */
/** Keep this test helper uninlined so acceptance exercises the permitted C stack boundary. */
__attribute__((noinline)) static CHILD(ApiResult) AMA_NATIVE(call_child)(struct AMA_NATIVE(Child) *child,
    const CHILD_TEXT(Borrowed) *input,CHILD_TEXT(Value) **response,CHILD_CONDITION(Value) **previous) {
    AMA_EMBED_BEFORE_CHILD(child->context,child->instance);
    CHILD(ApiResult) result=child->outcome==CHILD_YIELDED
        ? CHILD_EXPORT(resume)(child->context,child->instance,response,previous,&child->outcome)
        : CHILD_EXPORT(entry)(child->context,child->instance,input,response,previous,&child->outcome);
    AMA_EMBED_AFTER_CHILD(child->context,child->instance);
    return result;
}
