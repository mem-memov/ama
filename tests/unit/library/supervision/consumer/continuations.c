/** @file
 * @brief Exercise independent parent/child saved positions and abandonment with all C frames returned.
 */
/** Retained reads and a sibling execution cannot resume another parent's child implicitly. */
static void it_returns_all_c_frames_before_each_explicit_outer_yield(void) {
    reset_fixture();
    PARENT(Context) context={0}; PARENT(Instance) first={0},second={0};
    parent_ok(PARENT(context_create)(&context));
    parent_ok(PARENT_EXPORT(create)(context,&first)); parent_ok(PARENT_EXPORT(create)(context,&second));
    assert(created==2 && resources==4 && signals==0 && steps==0);
    other_root=second;
    Reply old={0},next={0}; uint32_t outcome=0;
    parent_ok(enter(context,first,"first",false,true,&old,&outcome)); assert(outcome==2);
    reply_is(&old,"first",false,true,NULL); assert(steps==1 && signals==1);
    parent_ok(PARENT_EXPORT(read)(context,first,&next.text,&next.previous,&next.waiting,&next.failure));
    reply_is(&next,"first",false,true,NULL); release_reply(&next); assert(steps==1 && signals==1);
    other_root=first;
    parent_ok(enter(context,second,"second",false,true,&next,&outcome)); assert(outcome==2);
    reply_is(&next,"second",false,true,NULL); release_reply(&next); assert(steps==2 && signals==2);
    other_root=second;
    parent_ok(resume(context,first,&next,&outcome)); assert(outcome==2);
    reply_is(&next,"first",true,true,NULL); release_reply(&next);
    parent_ok(resume(context,first,&next,&outcome)); assert(outcome==1);
    reply_is(&next,"first",true,false,NULL); release_reply(&next);
    assert(steps==4 && signals==4);
    parent_ok(enter(context,first,"new request",false,true,&next,&outcome)); assert(outcome==2);
    reply_is(&next,"new request",true,true,NULL); release_reply(&next);
    size_t calls=steps,callbacks=signals;
    parent_ok(PARENT(context_dispose)(&context)); returned();
    assert(created==disposed && resources==0 && steps==calls && signals==callbacks);
    assert(CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==2);
    reply_is(&old,"first",false,true,NULL); release_reply(&old); assert(live()==0);
}
