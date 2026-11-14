/** @file
 * @brief Assert actual C entry/resume execution, native reentry, ownership and failure isolation.
 */
#include "consumer-api.h"
/* Assertions belong to this test consumer even when its component is built with NDEBUG. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
/** Independently owned public results; every destination starts empty. */
typedef struct {
    TEXT(Value) *text, *copy;
    NUMBER(Value) *number;
    CONDITION(Value) *condition;
    FAILURE(Value) *failure;
    LIST(Value) *list;
} Snapshots;
static API(Context) callback_context;
static API(Instance) callback_instance, callback_peer;
static size_t native_calls, rejected_callbacks;
static int enable_reentry;
/** Consume the returned independent diagnostic after requiring API admission success. */
static void ok(API(ApiResult) result) {
    assert(result.code == 0);
    uint32_t released = API(result_release)(&result); assert(released == 0);
}
/** Native C helpers may try another host call, which must reject before executing/resetting anything. */
void fixture_reenter(void) {
    native_calls++;
    if (!enable_reentry) { return; }
    CONDITION(Value) *output = NULL; uint32_t outcome = 99;
    API(ApiResult) result = COUNTER(entry)(callback_context,callback_instance,NULL,&output,&outcome);
    assert(result.code == 2 && outcome == 99 && output == NULL);
    assert(API(result_release)(&result) == 0);
    result = COUNTER(read)(callback_context,callback_instance,&output);
    assert(result.code == 2 && output == NULL); assert(API(result_release)(&result) == 0);
    result = EXPORTED(read)(callback_context,callback_peer,NULL,NULL,NULL,NULL,NULL,NULL);
    assert(result.code == 2); assert(API(result_release)(&result) == 0);
    API(Diagnostic) diagnostic = {0}; result = API(diagnostic)(callback_context,&diagnostic);
    assert(result.code == 2 && diagnostic.code == 0 && diagnostic.message == NULL); assert(API(result_release)(&result) == 0);
    API(Context) context = callback_context; result = API(context_dispose)(&context);
    assert(result.code == 2 && context.opaque[0] == callback_context.opaque[0] && context.opaque[1] == callback_context.opaque[1]);
    assert(API(result_release)(&result) == 0);
    API(Instance) instance = callback_instance; result = API(instance_dispose)(callback_context,&instance);
    assert(result.code == 2 && instance.opaque[0] == callback_instance.opaque[0] && instance.opaque[1] == callback_instance.opaque[1] && instance.opaque[2] == callback_instance.opaque[2]);
    assert(API(result_release)(&result) == 0); rejected_callbacks++;
}
/** Release each snapshot exactly once and leave reusable empty output slots. */
static void release(Snapshots *values) {
    ok(TEXT(release)(&values->text)); ok(TEXT(release)(&values->copy));
    ok(NUMBER(release)(&values->number)); ok(CONDITION(release)(&values->condition));
    ok(FAILURE(release)(&values->failure)); ok(LIST(release)(&values->list));
}
/** Construct a context and explicitly selected Ama root; its body is not executed. */
static void create(API(Context) *context, API(Instance) *instance) {
    ok(API(context_create)(context)); ok(EXPORTED(create)(*context,instance));
}
/** Supply owned mathematical/UTF-8/failure/list values and select all public outputs. */
static API(ApiResult) entry(API(Context) context, API(Instance) instance, Snapshots *values, uint32_t *outcome) {
    static const uint8_t bytes[] = {97,0,208,176}, magnitude[] = {1,0,254};
    TEXT(Borrowed) text = {bytes,sizeof(bytes)}, items[] = {text,text};
    NUMBER(Borrowed) number = {1,{magnitude,sizeof(magnitude)}}; CONDITION(Borrowed) condition = {1};
    FAILURE(Borrowed) failure = {1,{bytes,sizeof(bytes)}}; LIST(Borrowed) list = {items,2};
    return EXPORTED(entry)(context,instance,&text,&number,&condition,&failure,&list,
        values == NULL ? NULL : &values->text,values == NULL ? NULL : &values->number,
        values == NULL ? NULL : &values->condition,values == NULL ? NULL : &values->failure,
        values == NULL ? NULL : &values->list,values == NULL ? NULL : &values->copy,outcome);
}
/** Resume supplies no new inputs and selects the same independent output schemas. */
static API(ApiResult) resume(API(Context) context, API(Instance) instance, Snapshots *values, uint32_t *outcome) {
    return EXPORTED(resume)(context,instance,&values->text,&values->number,&values->condition,
        &values->failure,&values->list,&values->copy,outcome);
}
/** Read a native method flag without invoking or resetting its native body state. */
static unsigned flag(const CONDITION(Value) *value) {
    CONDITION(Borrowed) view = {0}; ok(CONDITION(view)(value,&view)); return view.value;
}
/** Inspect bytes/condition after the original context changed or was disposed. */
static void expected(const Snapshots *values, unsigned condition) {
    static const uint8_t bytes[] = {97,0,208,176}, magnitude[] = {1,0,254};
    TEXT(Borrowed) text = {0}; NUMBER(Borrowed) number = {0}; FAILURE(Borrowed) failure = {0}; LIST(Borrowed) list = {0};
    ok(TEXT(view)(values->text,&text)); assert(text.length == sizeof(bytes) && memcmp(text.data,bytes,sizeof(bytes)) == 0);
    ok(NUMBER(view)(values->number,&number));
    assert(number.negative == 1 && number.magnitude.length == sizeof(magnitude) && memcmp(number.magnitude.data,magnitude,sizeof(magnitude)) == 0);
    assert(flag(values->condition) == condition);
    ok(FAILURE(view)(values->failure,&failure));
    assert(failure.failed == 1 && failure.description.length == sizeof(bytes) && memcmp(failure.description.data,bytes,sizeof(bytes)) == 0);
    ok(LIST(view)(values->list,&list)); assert(list.length == 2 && memcmp(list.items[1].data,bytes,sizeof(bytes)) == 0);
}
/** Unavailable resume and invalid destination regions reject without touching outcome or stored results. */
static void it_rejects_invalid_modes_controls_and_overlapping_outputs_before_execution(void) {
    API(Context) context = {0}, other = {0}; API(Instance) instance = {0}, peer = {0};
    create(&context,&instance); create(&other,&peer);
    API(Instance) observer = {0}; ok(OBSERVER(create)(context,&observer));
    Snapshots values = {0}; uint32_t outcome = 99;
    ok(OBSERVER(read)(context,observer,&values.text,&values.number,&values.condition,&values.failure,&values.list,&values.copy));
    assert(flag(values.condition) == 0); release(&values);
    API(ApiResult) refused = entry(context,observer,&values,&outcome);
    assert(refused.code == 1 && outcome == 99); assert(API(result_release)(&refused) == 0);
    API(ApiResult) result = resume(context,instance,&values,&outcome);
    assert(result.code == 3 && outcome == 99 && values.text == NULL); assert(API(result_release)(&result) == 0);
    result = entry(other,instance,&values,&outcome);
    assert(result.code == 1 && outcome == 99 && values.text == NULL); assert(API(result_release)(&result) == 0);
    result = OBSERVER(read)(context,instance,&values.text,&values.number,&values.condition,&values.failure,&values.list,&values.copy);
    assert(result.code == 1 && values.text == NULL); assert(API(result_release)(&result) == 0);
    CONDITION(Value) *invalid = NULL;
    result = COUNTER(entry)(context,instance,NULL,&invalid,&outcome);
    assert(result.code == 1 && outcome == 99 && invalid == NULL); assert(API(result_release)(&result) == 0);
    TEXT(Value) *alias = NULL;
    result = EXPORTED(entry)(context,instance,NULL,NULL,NULL,NULL,NULL,&alias,NULL,NULL,NULL,NULL,&alias,&outcome);
    assert(result.code == 1 && outcome == 99 && alias == NULL); assert(API(result_release)(&result) == 0);
    union { TEXT(Value) *owner; uint32_t words[2]; } overlap = {.owner = NULL};
    result = EXPORTED(entry)(context,instance,NULL,NULL,NULL,NULL,NULL,&overlap.owner,NULL,NULL,NULL,NULL,NULL,&overlap.words[1]);
    assert(result.code == 1 && overlap.owner == NULL); assert(API(result_release)(&result) == 0);
    result = EXPORTED(entry)(context,instance,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    ok(entry(context,instance,&values,&outcome)); assert(outcome == 2); expected(&values,1);
    Snapshots untouched = {0}; result = entry(context,instance,&values,&outcome);
    assert(result.code == 1 && outcome == 2); assert(API(result_release)(&result) == 0);
    CONDITION(Borrowed) bad = {2}; outcome = 71;
    result = EXPORTED(entry)(context,instance,NULL,NULL,&bad,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&outcome);
    assert(result.code == 1 && outcome == 71); assert(API(result_release)(&result) == 0);
    ok(resume(context,instance,&untouched,&outcome)); assert(outcome == 1); expected(&untouched,0);
    release(&untouched); release(&values);
    ok(API(context_dispose)(&context)); ok(API(context_dispose)(&other)); assert(live_allocations() == 0);
}
/** Distinct exports dispatch their own bodies while native state persists across entry resets. */
static void it_yields_resumes_restarts_and_executes_distinct_export_roots(void) {
    API(Context) context = {0}; API(Instance) instance = {0}, counter = {0}; create(&context,&instance);
    ok(COUNTER(create)(context,&counter));
    CONDITION(Value) *initial = NULL; ok(COUNTER(read)(context,counter,&initial)); assert(flag(initial) == 0); ok(CONDITION(release)(&initial));
    size_t before = native_calls; assert(rejected_callbacks == 0);
    Snapshots first = {0}, second = {0}; uint32_t outcome = 99;
    ok(entry(context,instance,&first,&outcome)); assert(outcome == 2); expected(&first,1);
    callback_context = context; callback_instance = counter; callback_peer = instance; enable_reentry = 1;
    CONDITION(Value) *method = NULL; ok(COUNTER(entry)(context,counter,NULL,&method,&outcome));
    assert(outcome == 1 && flag(method) == 1 && native_calls == before + 1 && rejected_callbacks == 1); ok(CONDITION(release)(&method));
    ok(resume(context,instance,&second,&outcome)); assert(outcome == 1); expected(&second,0);
    Snapshots empty = {0}; outcome = 98;
    API(ApiResult) result = resume(context,instance,&empty,&outcome);
    assert(result.code == 3 && outcome == 98); assert(API(result_release)(&result) == 0);
    ok(COUNTER(entry)(context,counter,NULL,&method,&outcome)); assert(outcome == 1 && flag(method) == 0); ok(CONDITION(release)(&method));
    assert(native_calls == before + 2 && rejected_callbacks == 2);
    result = COUNTER(resume)(context,counter,&method,&outcome);
    assert(result.code == 3 && outcome == 1 && method == NULL); assert(API(result_release)(&result) == 0);
    release(&second);
    ok(entry(context,instance,&second,&outcome)); assert(outcome == 2); expected(&second,1);
    release(&second);
    ok(entry(context,instance,&second,&outcome)); assert(outcome == 2); expected(&second,1);
    enable_reentry = 0;
    ok(API(instance_dispose)(context,&instance)); ok(API(context_dispose)(&context));
    expected(&first,1); expected(&second,1); release(&first); release(&second);
    assert(live_allocations() == 0);
}
/** A native body/cleanup failure returns accepted FAILED and preserves the original cause in one context. */
static void it_keeps_technical_failure_and_cleanup_local_to_the_accepted_context(void) {
    API(Context) failed = {0}, healthy = {0}; API(Instance) counter = {0}, peer = {0};
    ok(API(context_create)(&failed)); ok(API(context_create)(&healthy));
    ok(COUNTER(create)(failed,&counter)); ok(COUNTER(create)(healthy,&peer));
    static const uint8_t bytes[] = "error-cleanup"; TEXT(Borrowed) input = {bytes,sizeof(bytes)-1};
    CONDITION(Value) *output = NULL; uint32_t outcome = 77;
    API(ApiResult) result = COUNTER(entry)(failed,counter,&input,&output,&outcome);
    assert(result.code == 0 && outcome == 3 && output == NULL && result.diagnostic.code != 0);
    static const char primary[] = "counter body failed";
    assert(result.diagnostic.length == sizeof(primary)-1 && memcmp(result.diagnostic.message,primary,sizeof(primary)-1) == 0);
    API(Diagnostic) diagnostic = {0}; ok(API(diagnostic)(failed,&diagnostic));
    assert(diagnostic.code == result.diagnostic.code && diagnostic.length == result.diagnostic.length);
    outcome = 76; API(ApiResult) denied = COUNTER(entry)(failed,counter,NULL,&output,&outcome);
    assert(denied.code != 0 && outcome == 76 && output == NULL); assert(API(result_release)(&denied) == 0);
    ok(COUNTER(entry)(healthy,peer,NULL,&output,&outcome)); assert(outcome == 1 && flag(output) == 1); ok(CONDITION(release)(&output));
    denied = API(context_dispose)(&failed); assert(denied.code == 5); assert(API(result_release)(&denied) == 0);
    assert(memcmp(diagnostic.message,primary,sizeof(primary)-1) == 0);
    assert(API(result_release)(&result) == 0 && API(diagnostic_release)(&diagnostic) == 0);
    ok(API(context_dispose)(&healthy)); assert(live_allocations() == 0);
}
/** Typed input rejection after admission is FAILED rather than a recoverable API refusal. */
static void it_reports_malformed_typed_input_as_accepted_failure(void) {
    API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance);
    static const uint8_t bytes[] = {192,128}; TEXT(Borrowed) malformed = {bytes,sizeof(bytes)};
    Snapshots values = {0}; uint32_t outcome = 74;
    API(ApiResult) result = EXPORTED(entry)(context,instance,&malformed,NULL,NULL,NULL,NULL,
        &values.text,&values.number,&values.condition,&values.failure,&values.list,&values.copy,&outcome);
    assert(result.code == 0 && outcome == 3 && result.diagnostic.code != 0 && values.text == NULL && values.failure == NULL);
    assert(API(result_release)(&result) == 0);
    result = API(context_dispose)(&context); assert(result.code == 5); assert(API(result_release)(&result) == 0);
    assert(live_allocations() == 0);
}
/** Fail every measured entry and resume allocation with no partial output publication or leaked state. */
static void it_unwinds_every_accepted_entry_and_resume_allocation_failure(void) {
    for (unsigned resuming = 0; resuming != 2; resuming++) {
        size_t count = 0;
        for (size_t offset = 0; offset <= count; offset++) {
            API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance);
            uint32_t outcome = 73;
            if (resuming) { ok(entry(context,instance,NULL,&outcome)); assert(outcome == 2); }
            Snapshots values = {0}; select_failure(offset);
            API(ApiResult) result = resuming ? resume(context,instance,&values,&outcome) : entry(context,instance,&values,&outcome);
            if (offset == 0) {
                assert(result.code == 0 && outcome == (resuming ? 1 : 2)); count = allocation_calls(); assert(count != 0);
                assert(API(result_release)(&result) == 0); release(&values);
            } else {
                assert(result.code == 0 && outcome == 3 && result.diagnostic.code != 0
                    && values.text == NULL && values.number == NULL && values.condition == NULL
                    && values.failure == NULL && values.list == NULL && values.copy == NULL);
                assert(API(result_release)(&result) == 0);
            }
            select_failure(0); result = API(context_dispose)(&context);
            assert(result.code == (offset == 0 ? 0 : 5)); assert(API(result_release)(&result) == 0);
            assert(live_allocations() == 0);
        }
    }
}
/** Exercise public entry/resume behavior through actual native bodies and stackless Ama labels. */
void run(void) {
    it_rejects_invalid_modes_controls_and_overlapping_outputs_before_execution();
    it_yields_resumes_restarts_and_executes_distinct_export_roots();
    it_keeps_technical_failure_and_cleanup_local_to_the_accepted_context();
    it_reports_malformed_typed_input_as_accepted_failure();
    it_unwinds_every_accepted_entry_and_resume_allocation_failure();
}
/** Return only after all explicit contexts, native resources, snapshots and diagnostics are consumed. */
int main(void) { run(); return 0; }
