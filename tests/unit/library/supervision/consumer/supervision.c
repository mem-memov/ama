/** @file
 * @brief Translate only a foreign failed context, preserving own failures and explicit terminal/reset policy.
 */
/** A copied report outlives child cleanup; later input cannot restart a terminal child implicitly. */
static void it_copies_child_reports_and_requires_explicit_reset_after_supervised_failure(void) {
    const char *requests[]={"fault","fault-cleanup"};
    for(size_t fault=0;fault<sizeof(requests)/sizeof(*requests);fault++) {
        reset_fixture();
        PARENT(Context) context={0}; PARENT(Instance) instance={0}; Reply report={0},next={0}; uint32_t outcome=0;
        parent_ok(PARENT(context_create)(&context)); parent_ok(PARENT_EXPORT(create)(context,&instance)); other_root=instance;
        parent_ok(enter(context,instance,requests[fault],false,false,&report,&outcome)); assert(outcome==1);
        reply_is(&report,"",false,false,"Deliberate child execution failure");
        assert(created==disposed && resources==0 && CHILD(code_can_unload)()==0 && steps==1);
        parent_ok(enter(context,instance,"new input is not a reset",false,false,&next,&outcome)); assert(outcome==1);
        reply_is(&next,"",false,false,"Child failed; explicit reset required"); release_reply(&next); assert(steps==1);
        parent_ok(enter(context,instance,"recovered",true,false,&next,&outcome)); assert(outcome==2);
        reply_is(&next,"recovered",false,true,NULL); release_reply(&next); assert(created==2 && disposed==1);
        parent_ok(resume(context,instance,&next,&outcome)); assert(outcome==2);
        reply_is(&next,"recovered",true,true,NULL); release_reply(&next);
        parent_ok(resume(context,instance,&next,&outcome)); assert(outcome==1);
        reply_is(&next,"recovered",true,false,NULL); release_reply(&next);
        parent_ok(PARENT(context_dispose)(&context)); assert(created==disposed && resources==0);
        reply_is(&report,"",false,false,"Deliberate child execution failure"); release_reply(&report); assert(live()==0);
    }
}
/** Deliberate propagation terminates the parent and consumes its other unexecuted child too. */
static void it_propagates_child_failure_only_when_the_parent_policy_requests_it(void) {
    reset_fixture();
    PARENT(Context) context={0}; PARENT(Instance) first={0},other={0}; Reply reply={0}; uint32_t outcome=0;
    parent_ok(PARENT(context_create)(&context)); parent_ok(PARENT_EXPORT(create)(context,&first)); parent_ok(PARENT_EXPORT(create)(context,&other));
    other_root=other;
    PARENT(ApiResult) result=enter(context,first,"fault-cleanup",false,true,&reply,&outcome);
    assert(result.code==0 && outcome==3 && reply.text==NULL && reply.previous==NULL && reply.waiting==NULL && reply.failure==NULL);
    assert(created==disposed && resources==0 && steps==1 && signals==1 && CHILD(code_can_unload)()==0);
    assert(result.diagnostic.length==sizeof("Child execution failed")-1);
    assert(memcmp(result.diagnostic.message,"Child execution failed",result.diagnostic.length)==0);
    assert(PARENT(result_release)(&result)==0);
    PARENT(Diagnostic) diagnostic={0}; parent_ok(PARENT(diagnostic)(context,&diagnostic));
    result=PARENT_EXPORT(read)(context,other,NULL,NULL,NULL,NULL); assert(result.code==1); assert(PARENT(result_release)(&result)==0);
    result=PARENT(context_dispose)(&context); assert(result.code==5 && context.opaque[0]==0); assert(PARENT(result_release)(&result)==0);
    assert(diagnostic.length==sizeof("Child execution failed")-1 && memcmp(diagnostic.message,"Child execution failed",diagnostic.length)==0);
    assert(PARENT(diagnostic_release)(&diagnostic)==0 && live()==0);
}
