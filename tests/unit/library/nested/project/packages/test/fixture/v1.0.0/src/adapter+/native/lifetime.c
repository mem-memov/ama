/** @file
 * @brief Own and unwind a fresh isolated child context and selected instance.
 * [EMBED-07](../../../../../../../../../../../../spec/implementation/c-library.md#embed-07) and
 * [EMBED-08](../../../../../../../../../../../../spec/implementation/c-library.md#embed-08) require
 * exclusive foreign ownership and normal C returns at every step boundary.
 */
/** Consume an API result before translating failure to immutable parent-owned text. */
static bool AMA_NATIVE(accepted)(CHILD(ApiResult) result) {
    bool accepted=result.code==0;
    uint32_t released=CHILD(result_release)(&result);
    return accepted && released==0;
}
/** Only the exclusive adapter can execute this child. Quiescent outer cleanup therefore
 * cannot encounter an in-flight child activation; a saved child position is never resumed here.
 */
static AmaStatus AMA_NATIVE(close_child)(struct AMA_NATIVE(Child) *child) {
    if (child->context.opaque[0]==0) { return ama_status_success(); }
    CHILD(Context) previous=child->context;
    CHILD(ApiResult) result=CHILD(context_dispose)(&child->context);
    bool consumed=child->context.opaque[0]==0;
    bool accepted=AMA_NATIVE(accepted)(result);
    if (consumed) {
        child->instance=(CHILD(Instance)){0};
        AMA_EMBED_DISPOSED(previous);
    }
    return consumed && accepted ? ama_status_success()
        : ama_status_make(AMA_NATIVE_FAILURE,"Child context cleanup failed",sizeof("Child context cleanup failed")-1);
}
/** Open only an empty holder; explicit reset and initialization share the same exact unwind. */
static AmaStatus AMA_NATIVE(open_child)(struct AMA_NATIVE(Child) *child) {
    if (child->context.opaque[0]!=0) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    bool created=AMA_NATIVE(accepted)(AMA_EMBED_CONTEXT_CREATE(&child->context));
    if (created) {
        AMA_EMBED_CREATED(child->context);
        created=AMA_NATIVE(accepted)(AMA_EMBED_CREATE(child->context,&child->instance));
    }
    if (!created) {
        /* Native initialization failure receives no destructor credit. Own this unwind,
         * including the context left terminal by an accepted failed child constructor. */
        (void)AMA_NATIVE(close_child)(child);
        return ama_status_make(AMA_NATIVE_FAILURE,"Child resource construction failed",sizeof("Child resource construction failed")-1);
    }
    child->outcome=0;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    state->child=NULL;
    struct AMA_NATIVE(Child) *child=malloc(sizeof(*child));
    if (child==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }
    *child=(struct AMA_NATIVE(Child)){0};
    AmaStatus status=AMA_NATIVE(open_child)(child);
    if (status.code!=AMA_OK) { free(child); return status; }
    state->child=child;
    return ama_status_success();
}
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    if (state->child==NULL) { return ama_status_success(); }
    struct AMA_NATIVE(Child) *child=state->child;
    state->child=NULL;
    AmaStatus status=AMA_NATIVE(close_child)(child);
    free(child);
    return status;
}
