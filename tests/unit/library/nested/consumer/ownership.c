/** @file
 * @brief Verify constructor-only factories, independent child instances and abandonment without resume.
 */
/** Child constants/defaults and retained state stay isolated while wire replies outlive both domains. */
static void it_owns_fresh_child_resources_and_copies_only_independent_results(void) {
    reset_fixture();
    PARENT(Context) context={0}; PARENT(Instance) first={0},second={0};
    parent_ok(PARENT(context_create)(&context));
    parent_ok(PARENT_EXPORT(create)(context,&first)); parent_ok(PARENT_EXPORT(create)(context,&second));
    assert(created==2 && disposed==0 && steps==0);
    for(size_t i=0;i<2;i++) {
        CHILD_TEXT(Value) *text=NULL; CHILD_CONDITION(Value) *previous=NULL;
        child_ok(CHILD_EXPORT(read)(children[i].context,children[i].instance,&text,&previous));
        CHILD_TEXT(Borrowed) bytes={0}; CHILD_CONDITION(Borrowed) condition={0};
        child_ok(CHILD_TEXT(view)(text,&bytes)); child_ok(CHILD_CONDITION(view)(previous,&condition));
        assert(bytes.length==13 && memcmp(bytes.data,"child factory",13)==0 && condition.value==0);
        child_ok(CHILD_TEXT(release)(&text)); child_ok(CHILD_CONDITION(release)(&previous));
    }
    other_root=second;
    Reply old={0},next={0}; char input[]="first";
    step(context,first,input,&old); reply_is(&old,"first",false,true);
    memset(input,'x',sizeof(input)-1);
    step(context,first,"ignored resume input",&next); reply_is(&next,"first",false,false); release_reply(&next);
    step(context,first,"third",&next); reply_is(&next,"third",true,true); release_reply(&next);
    step(context,first,"ignored again",&next); reply_is(&next,"third",true,false); release_reply(&next);
    const uint8_t expected[]={97,0,208,176}; uint8_t counted[]={97,0,208,176};
    PARENT_TEXT(Borrowed) input_bytes={counted,sizeof(counted)}; uint32_t outcome=0;
    parent_ok(PARENT_EXPORT(entry)(context,first,&input_bytes,&next.text,&next.previous,&next.waiting,&outcome));
    assert(outcome==1); memset(counted,0,sizeof(counted));
    PARENT_TEXT(Borrowed) output_bytes={0}; parent_ok(PARENT_TEXT(view)(next.text,&output_bytes));
    assert(output_bytes.length==sizeof(expected) && memcmp(output_bytes.data,expected,sizeof(expected))==0);
    release_reply(&next);
    other_root=first;
    step(context,second,"other",&next); reply_is(&next,"other",false,true); release_reply(&next);
    size_t calls=steps;
    parent_ok(PARENT(instance_dispose)(context,&first)); assert(created==2 && disposed==1 && steps==calls);
    parent_ok(PARENT(context_dispose)(&context)); assert(disposed==2 && steps==calls);
    CHILD(Diagnostic) stale={0}; CHILD(ApiResult) denied=CHILD(diagnostic)(last_disposed,&stale);
    assert(denied.code==1 && stale.code==0); assert(CHILD(result_release)(&denied)==0);
    assert(CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==2);
    reply_is(&old,"first",false,true); release_reply(&old);
    assert(live()==0 && PARENT(code_can_unload)()==0);
}
