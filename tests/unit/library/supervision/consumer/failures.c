/** @file
 * @brief Require report allocation failure to fail the parent rather than manufacture application success.
 */
/** Every measured pre-entry, report-copy and snapshot failure consumes all foreign resources. */
static void it_keeps_own_report_allocation_failures_technical(void) {
    size_t count=0;
    for(size_t failure=0;;failure++) {
        reset_fixture();
        PARENT(Context) context={0}; PARENT(Instance) instance={0}; Reply reply={0}; uint32_t outcome=0;
        parent_ok(PARENT(context_create)(&context)); parent_ok(PARENT_EXPORT(create)(context,&instance)); other_root=instance;
        select_failure(failure);
        PARENT(ApiResult) result=enter(context,instance,"fault",false,false,&reply,&outcome);
        if(failure==0) { count=allocations(); assert(count!=0); }
        assert(result.code==0 && outcome==(failure==0 ? 1u : 3u)); assert(PARENT(result_release)(&result)==0);
        if(failure==0) { reply_is(&reply,"",false,false,"Deliberate child execution failure"); release_reply(&reply); }
        else { assert(reply.text==NULL && reply.previous==NULL && reply.waiting==NULL && reply.failure==NULL); }
        result=PARENT(context_dispose)(&context); assert(result.code==(failure==0 ? 0u : 5u)); assert(PARENT(result_release)(&result)==0);
        assert(created==disposed && resources==0 && live()==0 && CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==0);
        if(failure==count) { break; }
    }
}
