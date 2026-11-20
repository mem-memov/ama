/** @file
 * @brief Keep an independent peer context alive when a template-backed owned child is abandoned or fails.
 */
/** Dispose A and its exclusively owned B, while the unrelated context C and its
 * own child retain their saved computation. No cleanup executes another step. */
static void it_preserves_independent_peers_during_owned_child_cleanup(void) {
    for (unsigned failure=0;failure<2;failure++) {
        reset_fixture();
        PARENT(Context) outer={0},peer={0}; PARENT(Instance) first={0},second={0};
        Reply reply={0}; uint32_t outcome=0;
        parent_ok(PARENT(context_create)(&outer)); parent_ok(PARENT_EXPORT(create)(outer,&first));
        parent_ok(PARENT(context_create)(&peer)); parent_ok(PARENT_EXPORT(create)(peer,&second));
        other_root=second;
        parent_ok(enter(peer,second,"peer",false,true,&reply,&outcome)); assert(outcome==2);
        reply_is(&reply,"peer",false,true,NULL); release_reply(&reply);
        other_root=first;
        parent_ok(enter(outer,first,failure ? "fault" : "abandoned",false,true,&reply,&outcome));
        assert(outcome==(failure ? 3u : 2u));
        if (!failure) { reply_is(&reply,"abandoned",false,true,NULL); release_reply(&reply); }
        size_t calls=steps,callbacks=signals;
        PARENT(ApiResult) disposed_outer=PARENT(context_dispose)(&outer);
        assert(disposed_outer.code==(failure ? 5u : 0u)); assert(PARENT(result_release)(&disposed_outer)==0);
        assert(disposed==1 && created==2 && resources==2 && steps==calls && signals==callbacks);
        other_root=second;
        parent_ok(resume(peer,second,&reply,&outcome)); assert(outcome==2);
        reply_is(&reply,"peer",true,true,NULL); release_reply(&reply);
        parent_ok(resume(peer,second,&reply,&outcome)); assert(outcome==1);
        reply_is(&reply,"peer",true,false,NULL); release_reply(&reply);
        parent_ok(PARENT(context_dispose)(&peer));
        assert(created==disposed && resources==0 && live()==0);
        assert(CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==0);
    }
}
