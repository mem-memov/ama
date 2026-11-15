/** @file
 * @brief Observe nested C intervals and both active context gates from public headers only.
 */
#ifndef AMA_SUPERVISION_CONSUMER_COMMON
#define AMA_SUPERVISION_CONSUMER_COMMON
#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "parent-api.h"
#include "child-api.h"
#include "../probe.h"
/** Reset measured parent allocation failure selection. */
void select_failure(size_t);
/** Return parent allocator calls since the last selection. */
size_t allocations(void);
/** Return live parent allocations, excluding independent child resources. */
size_t live(void);
/** The observer borrows checked controls; the adapter remains the sole context disposal owner. */
typedef struct { CHILD(Context) context; CHILD(Instance) primary,other; bool live; } ChildRecord;
static ChildRecord children[8];
static size_t created,disposed,steps,signals,resources,parent_depth,child_depth;
static PARENT(Context) current_parent;
static PARENT(Instance) current_root,other_root;
static ChildRecord *current_child;
/** Consume parent result diagnostics independently of execution/application outcomes. */
static void parent_ok(PARENT(ApiResult) result) {
    assert(result.code==0); assert(PARENT(result_release)(&result)==0);
}
/** Child owners have their own releasing component boundary. */
static void child_ok(CHILD(ApiResult) result) {
    assert(result.code==0); assert(CHILD(result_release)(&result)==0);
}
/** Locate a live checked context without ever inspecting private child memory. */
static ChildRecord *record(CHILD(Context) context) {
    for(size_t i=0;i<sizeof(children)/sizeof(*children);i++) {
        if(children[i].live && children[i].context.opaque[0]==context.opaque[0]
            && children[i].context.opaque[1]==context.opaque[1]) { return &children[i]; }
    }
    assert(false); return NULL;
}
/** Construct constants only; no implicit root or native entry is executed. */
CHILD(ApiResult) fixture_context_create(CHILD(Context) *context) { return CHILD(context_create)(context); }
/** Add an idle sibling root so active-context checks cannot be bypassed through another instance. */
CHILD(ApiResult) fixture_instance_create(CHILD(Context) context,CHILD(Instance) *instance) {
    CHILD(ApiResult) result=CHILD_EXPORT(create)(context,instance);
    if(result.code!=0) { return result; }
    child_ok(result); record(context)->primary=*instance;
    return CHILD_EXPORT(create)(context,&record(context)->other);
}
/** Observe one exclusive acquisition without acquiring another destruction responsibility. */
void fixture_created(CHILD(Context) context) {
    for(size_t i=0;i<sizeof(children)/sizeof(*children);i++) {
        if(!children[i].live) { children[i]=(ChildRecord){.context=context,.live=true}; created++; return; }
    }
    assert(false);
}
/** Clear borrowed controls only after the public context token was consumed. */
void fixture_disposed(CHILD(Context) context) { record(context)->live=false; disposed++; }
/** Prove the active outer Ama context rejects another root and ancestor disposal. */
static void parent_is_busy(void) {
    PARENT(ApiResult) denied=PARENT_EXPORT(read)(current_parent,other_root,NULL,NULL,NULL,NULL);
    assert(denied.code==2); assert(PARENT(result_release)(&denied)==0);
    uint32_t outcome=99;
    denied=PARENT_EXPORT(resume)(current_parent,current_root,NULL,NULL,NULL,NULL,&outcome);
    assert(denied.code==2 && outcome==99); assert(PARENT(result_release)(&denied)==0);
    PARENT(Context) copied=current_parent;
    denied=PARENT(context_dispose)(&copied);
    assert(denied.code==2 && copied.opaque[0]==current_parent.opaque[0]); assert(PARENT(result_release)(&denied)==0);
}
/** Enter the actual uninlined parent C helper interval. */
void fixture_before_child(CHILD(Context) context,CHILD(Instance) instance) {
    assert(parent_depth==0 && child_depth==0); parent_depth=1; steps++;
    current_child=record(context);
    assert(current_child->primary.opaque[2]==instance.opaque[2]); parent_is_busy();
}
/** A child return has released every child helper/lease before the outer native entry can finish. */
void fixture_after_child(CHILD(Context) context,CHILD(Instance) instance) {
    (void)context; (void)instance;
    assert(parent_depth==1 && child_depth==0); parent_depth=0;
}
/** The callback executes C assertions only; every attempted Ama operation is rejected while active. */
static void observe(uint32_t event) {
    if(event==AMA_NESTED_RESOURCE_CREATE) { resources++; return; }
    if(event==AMA_NESTED_RESOURCE_DESTROY) { assert(resources!=0); resources--; return; }
    assert(parent_depth==1 && current_child!=NULL);
    if(event==AMA_NESTED_HELPER_ENTER) {
        assert(child_depth==0); child_depth=1; signals++;
        parent_is_busy();
        CHILD(ApiResult) denied=CHILD_EXPORT(read)(current_child->context,current_child->other,NULL,NULL);
        assert(denied.code==2); assert(CHILD(result_release)(&denied)==0);
        uint32_t outcome=97;
        denied=CHILD_EXPORT(resume)(current_child->context,current_child->primary,NULL,NULL,&outcome);
        assert(denied.code==2 && outcome==97); assert(CHILD(result_release)(&denied)==0);
        CHILD(Context) copied=current_child->context;
        denied=CHILD(context_dispose)(&copied);
        assert(denied.code==2 && copied.opaque[0]==current_child->context.opaque[0]); assert(CHILD(result_release)(&denied)==0);
    } else {
        assert(event==AMA_NESTED_HELPER_LEAVE && child_depth==1); child_depth=0;
    }
}
/** Returned host calls never retain an ordinary C helper activation. */
static void returned(void) { assert(parent_depth==0 && child_depth==0); }
/** Reset only after every previous context/resource/value owner was retired. */
static void reset_fixture(void) {
    returned(); assert(created==disposed && resources==0 && live()==0);
    assert(CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==0);
    memset(children,0,sizeof(children)); created=disposed=steps=signals=0;
    current_parent=(PARENT(Context)){0}; current_root=other_root=(PARENT(Instance)){0}; current_child=NULL;
    select_failure(0); ama_nested_probe_v1_set(observe);
}
/** Own retained parent transport values independently of either execution domain. */
typedef struct { PARENT_TEXT(Value) *text; PARENT_CONDITION(Value) *previous,*waiting; PARENT_FAILURE(Value) *failure; } Reply;
/** Consume all initialized owners, including replies that survive child/parent cleanup. */
static void release_reply(Reply *reply) {
    parent_ok(PARENT_TEXT(release)(&reply->text)); parent_ok(PARENT_CONDITION(release)(&reply->previous));
    parent_ok(PARENT_CONDITION(release)(&reply->waiting)); parent_ok(PARENT_FAILURE(release)(&reply->failure));
}
/** Check coherent condition/description without using diagnostic text to select behavior. */
static void reply_is(const Reply *reply,const char *text,bool previous,bool waiting,const char *failure) {
    PARENT_TEXT(Borrowed) bytes={0}; PARENT_CONDITION(Borrowed) a={0},b={0}; PARENT_FAILURE(Borrowed) error={0};
    parent_ok(PARENT_TEXT(view)(reply->text,&bytes)); parent_ok(PARENT_CONDITION(view)(reply->previous,&a));
    parent_ok(PARENT_CONDITION(view)(reply->waiting,&b)); parent_ok(PARENT_FAILURE(view)(reply->failure,&error));
    assert(bytes.length==strlen(text));
    if(bytes.length!=0) { assert(memcmp(bytes.data,text,bytes.length)==0); }
    assert(a.value==(uint8_t)previous && b.value==(uint8_t)waiting);
    assert(error.failed==(failure!=NULL));
    assert(error.description.length==(failure==NULL ? 0 : strlen(failure)));
    if(error.description.length!=0) { assert(memcmp(error.description.data,failure,error.description.length)==0); }
}
/** Explicit entry supplies policy; resume below supplies no new input or policy values. */
static PARENT(ApiResult) enter(PARENT(Context) context,PARENT(Instance) instance,const char *text,
    bool reset,bool propagate,Reply *reply,uint32_t *outcome) {
    current_parent=context; current_root=instance;
    if(other_root.opaque[0]==0) { other_root=instance; }
    PARENT_TEXT(Borrowed) input={(const uint8_t *)text,strlen(text)};
    PARENT_CONDITION(Borrowed) restarting={(uint8_t)reset},policy={(uint8_t)propagate};
    PARENT(ApiResult) result=PARENT_EXPORT(entry)(context,instance,&input,&restarting,&policy,
        &reply->text,&reply->previous,&reply->waiting,&reply->failure,outcome);
    returned(); return result;
}
/** Resume the independent outer Ama continuation; the adapter separately chooses child resume. */
static PARENT(ApiResult) resume(PARENT(Context) context,PARENT(Instance) instance,Reply *reply,uint32_t *outcome) {
    current_parent=context; current_root=instance;
    PARENT(ApiResult) result=PARENT_EXPORT(resume)(context,instance,&reply->text,&reply->previous,&reply->waiting,&reply->failure,outcome);
    returned(); return result;
}
#endif
