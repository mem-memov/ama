/** @file
 * @brief Observe exclusive foreign controls through public headers without borrowing Ama storage.
 */
#ifndef AMA_NESTED_CONSUMER_COMMON
#define AMA_NESTED_CONSUMER_COMMON
#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "parent-api.h"
#include "child-api.h"
/** Parent-only allocation probe; the child archive has its own allocator/code registry. */
void select_failure(size_t);
/** Return the number of parent allocator calls since the last failure selection. */
size_t allocations(void);
/** Return outstanding parent allocations; child ownership is checked independently. */
size_t live(void);
/** Copied checked controls grant observation only, never a second disposal owner. */
typedef struct { CHILD(Context) context; CHILD(Instance) instance; bool live; } ChildRecord;
static ChildRecord children[8];
static size_t created,disposed,steps;
static unsigned creation_fault;
static CHILD(Context) last_disposed;
static PARENT(Context) active_parent;
static PARENT(Instance) active_root,other_root;
/** Consume every returned diagnostic even for successful public operations. */
static void parent_ok(PARENT(ApiResult) result) {
    assert(result.code==0); assert(PARENT(result_release)(&result)==0);
}
/** Child results have their own independent releasing API and code lifetime. */
static void child_ok(CHILD(ApiResult) result) {
    if(result.code!=0) {
        fprintf(stderr,"Child API result %u, cause %u: %.*s\n",(unsigned)result.code,
            (unsigned)result.diagnostic.code,(int)result.diagnostic.length,result.diagnostic.message==NULL ? "" : (const char *)result.diagnostic.message);
    }
    assert(result.code==0); assert(CHILD(result_release)(&result)==0);
}
/** Match exact live numeric controls; no pointer or native state enters this observer. */
static ChildRecord *child_record(CHILD(Context) context) {
    for(size_t i=0;i<sizeof(children)/sizeof(*children);i++) {
        if(children[i].live && memcmp(&children[i].context,&context,sizeof(context))==0) { return &children[i]; }
    }
    assert(false); return NULL;
}
/** Force an actual checked child-factory refusal without publishing a context. */
CHILD(ApiResult) fixture_context_create(CHILD(Context) *context) {
    return CHILD(context_create)(creation_fault==1 ? NULL : context);
}
/** Select a genuine failing child constructor or a checked shape refusal for unwind coverage. */
CHILD(ApiResult) fixture_instance_create(CHILD(Context) context, CHILD(Instance) *instance) {
    if(creation_fault==2) { return CHILD_BROKEN(create)(context,instance); }
    if(creation_fault==3) {
        CHILD(Instance) nonempty={{1,0,0}};
        return CHILD_EXPORT(create)(context,&nonempty);
    }
    CHILD(ApiResult) result=CHILD_EXPORT(create)(context,instance);
    if(result.code==0) { child_record(context)->instance=*instance; }
    return result;
}
/** Register a borrowed observation only after the adapter acquired the real resource. */
void fixture_created(CHILD(Context) context) {
    for(size_t i=0;i<sizeof(children)/sizeof(*children);i++) {
        if(!children[i].live) {
            children[i]=(ChildRecord){.context=context,.live=true}; created++; return;
        }
    }
    assert(false);
}
/** The parent destructor is the sole disposal owner, including terminal child contexts. */
void fixture_disposed(CHILD(Context) context) {
    ChildRecord *record=child_record(context); record->live=false; disposed++; last_disposed=context;
}
/** Reject active parent/ancestor access and foreign instances before touching any graph. */
void fixture_before_child(CHILD(Context) context, CHILD(Instance) instance) {
    steps++;
    assert(active_parent.opaque[0]!=0);
    PARENT_TEXT(Value) *snapshot=NULL;
    PARENT(ApiResult) denied=PARENT_EXPORT(read)(active_parent,other_root,&snapshot,NULL,NULL);
    assert(denied.code==2 && snapshot==NULL); assert(PARENT(result_release)(&denied)==0);
    uint32_t outcome=91;
    denied=PARENT_EXPORT(entry)(active_parent,active_root,NULL,NULL,NULL,NULL,&outcome);
    assert(denied.code==2 && outcome==91); assert(PARENT(result_release)(&denied)==0);
    PARENT(Context) copied=active_parent;
    denied=PARENT(context_dispose)(&copied);
    assert(denied.code==2 && copied.opaque[0]==active_parent.opaque[0]); assert(PARENT(result_release)(&denied)==0);
    CHILD(Instance) foreign={{active_root.opaque[0],active_root.opaque[1],active_root.opaque[2]}};
    CHILD(ApiResult) wrong=CHILD_EXPORT(entry)(context,foreign,NULL,NULL,NULL,&outcome);
    assert(wrong.code==1 && outcome==91); assert(CHILD(result_release)(&wrong)==0);
    PARENT(Context) foreign_context={{context.opaque[0],context.opaque[1]}};
    PARENT(Instance) foreign_instance={{instance.opaque[0],instance.opaque[1],instance.opaque[2]}};
    denied=PARENT_EXPORT(read)(foreign_context,foreign_instance,NULL,NULL,NULL);
    assert(denied.code==1); assert(PARENT(result_release)(&denied)==0);
}
/** Reset only after every exclusive child resource and parent allocation was consumed. */
static void reset_fixture(void) {
    assert(created==disposed && live()==0);
    assert(CHILD(code_can_unload)()==0 && PARENT(code_can_unload)()==0);
    memset(children,0,sizeof(children));
    created=disposed=steps=0; creation_fault=0;
    active_parent=(PARENT(Context)){0}; active_root=other_root=(PARENT(Instance)){0};
    select_failure(0);
}
/** Own three independently releasing parent snapshots. */
typedef struct { PARENT_TEXT(Value) *text; PARENT_CONDITION(Value) *previous,*waiting; } Reply;
/** Consume every output owner, including replies that outlive all child contexts. */
static void release_reply(Reply *reply) {
    parent_ok(PARENT_TEXT(release)(&reply->text));
    parent_ok(PARENT_CONDITION(release)(&reply->previous));
    parent_ok(PARENT_CONDITION(release)(&reply->waiting));
}
/** Assert copied bytes and separately declared condition values without private payload access. */
static void reply_is(const Reply *reply, const char *text, bool previous, bool waiting) {
    PARENT_TEXT(Borrowed) bytes={0}; PARENT_CONDITION(Borrowed) a={0},b={0};
    parent_ok(PARENT_TEXT(view)(reply->text,&bytes));
    parent_ok(PARENT_CONDITION(view)(reply->previous,&a)); parent_ok(PARENT_CONDITION(view)(reply->waiting,&b));
    assert(bytes.length==strlen(text) && memcmp(bytes.data,text,bytes.length)==0);
    assert(a.value==(uint8_t)previous && b.value==(uint8_t)waiting);
}
/** Execute one explicit native adapter step; a yielded child is still a completed parent call. */
static void step(PARENT(Context) context, PARENT(Instance) instance, const char *text, Reply *reply) {
    active_parent=context; active_root=instance;
    PARENT_TEXT(Borrowed) input={(const uint8_t *)text,strlen(text)}; uint32_t outcome=0;
    parent_ok(PARENT_EXPORT(entry)(context,instance,&input,&reply->text,&reply->previous,&reply->waiting,&outcome));
    assert(outcome==1);
}
#endif
