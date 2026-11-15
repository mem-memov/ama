/** @file
 * @brief Exercise real child constructor failure and every measured parent ownership allocation failure.
 */
/** Rejected context/instance creation and accepted child constructor failure unwind one exclusive owner. */
static void it_unwinds_partial_child_resources_before_failed_parent_construction_returns(void) {
    for(unsigned fault=1;fault<=3;fault++) {
        reset_fixture(); creation_fault=fault;
        PARENT(Context) context={0}; PARENT(Instance) instance={0};
        parent_ok(PARENT(context_create)(&context));
        PARENT(ApiResult) result=PARENT_EXPORT(create)(context,&instance);
        assert(result.code==5 && instance.opaque[0]==0 && instance.opaque[1]==0 && instance.opaque[2]==0);
        assert(created==(fault==1 ? 0u : 1u) && disposed==created && steps==0);
        assert(CHILD(code_can_unload)()==0);
        PARENT(Diagnostic) diagnostic={0}; parent_ok(PARENT(diagnostic)(context,&diagnostic));
        assert(diagnostic.length==sizeof("Child resource construction failed")-1);
        PARENT(ApiResult) cleanup=PARENT(context_dispose)(&context);
        assert(cleanup.code==5 && context.opaque[0]==0);
        assert(PARENT(result_release)(&cleanup)==0 && PARENT(result_release)(&result)==0);
        assert(memcmp(diagnostic.message,"Child resource construction failed",diagnostic.length)==0);
        assert(PARENT(diagnostic_release)(&diagnostic)==0 && live()==0);
    }
}
/** Parent allocations before/after nested execution cannot leak the independently owned child. */
static void it_unwinds_every_parent_constructor_and_step_allocation_failure(void) {
    size_t constructor_count=0,step_count=0;
    for(unsigned operation=0;operation<2;operation++) {
        for(size_t failure=0;;failure++) {
            reset_fixture();
            PARENT(Context) context={0}; PARENT(Instance) instance={0};
            parent_ok(PARENT(context_create)(&context));
            if(operation==0) { select_failure(failure); }
            PARENT(ApiResult) result=PARENT_EXPORT(create)(context,&instance);
            if(operation==0) {
                if(failure==0) { constructor_count=allocations(); assert(constructor_count!=0); }
                assert(result.code==(failure==0 ? 0u : 5u));
                assert(PARENT(result_release)(&result)==0);
            } else {
                parent_ok(result); active_parent=context; active_root=other_root=instance;
                select_failure(failure);
                PARENT_TEXT(Borrowed) input={(const uint8_t *)"message",7}; Reply reply={0}; uint32_t outcome=99;
                result=PARENT_EXPORT(entry)(context,instance,&input,&reply.text,&reply.previous,&reply.waiting,&outcome);
                if(failure==0) { step_count=allocations(); assert(step_count!=0); }
                assert(result.code==0 && outcome==(failure==0 ? 1u : 3u));
                assert(PARENT(result_release)(&result)==0);
                if(failure==0) { reply_is(&reply,"message",false,true); release_reply(&reply); }
                else { assert(reply.text==NULL && reply.previous==NULL && reply.waiting==NULL); }
            }
            result=PARENT(context_dispose)(&context);
            assert(result.code==(failure==0 ? 0u : 5u) && context.opaque[0]==0);
            assert(PARENT(result_release)(&result)==0);
            assert(created==disposed && live()==0 && CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==0);
            if(failure==(operation==0 ? constructor_count : step_count)) { break; }
        }
    }
}
