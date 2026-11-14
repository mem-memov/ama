/** @file
 * @brief Assert actual generated C transport semantics without private compiler/runtime headers.
 */
#include "consumer-api.h"
#include <assert.h>
#include <string.h>
/** Independent public owner slots; no field contains Ama storage or an identity to dereference. */
typedef struct {
    TEXT(Value) *text, *copy;
    NUMBER(Value) *number;
    CONDITION(Value) *condition;
    FAILURE(Value) *failure;
    LIST(Value) *list;
} Snapshots;
/** Require technically successful API completion and consume its diagnostic owner. */
static void ok(API(ApiResult) result) {
    assert(result.code == 0);
    uint32_t released = API(result_release)(&result);
    assert(released == 0);
}
/** Consume every result independently, including initialized-empty skipped owners. */
static void release(Snapshots *values) {
    ok(TEXT(release)(&values->text)); ok(TEXT(release)(&values->copy));
    ok(NUMBER(release)(&values->number)); ok(CONDITION(release)(&values->condition));
    ok(FAILURE(release)(&values->failure)); ok(LIST(release)(&values->list));
}
/** Ask for all six outputs; the generated operation publishes them only together. */
static API(ApiResult) read_all(API(Context) context, API(Instance) instance, Snapshots *values) {
    return EXPORTED(read)(context,instance,&values->text,&values->number,&values->condition,
        &values->failure,&values->list,&values->copy);
}
/** Construct an explicitly selected instance without running its body. */
static void create(API(Context) *context, API(Instance) *instance) {
    ok(API(context_create)(context)); ok(EXPORTED(create)(*context,instance));
}
/** Verify every profile's mathematical/byte value after source storage was mutated or freed. */
static void expected_values(const Snapshots *values) {
    static const uint8_t bytes[] = {97,0,208,176}, magnitude[] = {1,0,254};
    TEXT(Borrowed) text = {0}, copy = {0}; NUMBER(Borrowed) number = {0};
    CONDITION(Borrowed) condition = {0}; FAILURE(Borrowed) failure = {0}; LIST(Borrowed) list = {0};
    ok(TEXT(view)(values->text,&text)); ok(TEXT(view)(values->copy,&copy));
    assert(text.length == sizeof(bytes) && copy.length == sizeof(bytes));
    assert(text.data != copy.data && memcmp(text.data,bytes,sizeof(bytes)) == 0 && memcmp(copy.data,bytes,sizeof(bytes)) == 0);
    ok(NUMBER(view)(values->number,&number));
    assert(number.negative == 1 && number.magnitude.length == sizeof(magnitude));
    assert(memcmp(number.magnitude.data,magnitude,sizeof(magnitude)) == 0);
    ok(CONDITION(view)(values->condition,&condition)); assert(condition.value == 1);
    ok(FAILURE(view)(values->failure,&failure));
    assert(failure.failed == 1 && failure.description.length == sizeof(bytes) && memcmp(failure.description.data,bytes,sizeof(bytes)) == 0);
    ok(LIST(view)(values->list,&list));
    assert(list.length == 2 && list.items[0].length == sizeof(bytes) && list.items[1].length == sizeof(bytes));
    assert(list.items[0].data != list.items[1].data && memcmp(list.items[1].data,bytes,sizeof(bytes)) == 0);
}
/** C input buffers are local borrows and can be overwritten immediately after preparation. */
static void prepare(API(Context) context, API(Instance) instance) {
    uint8_t bytes[] = {97,0,208,176}, magnitude[] = {1,0,254};
    TEXT(Borrowed) text = {bytes,sizeof(bytes)}, items[] = {text,text};
    NUMBER(Borrowed) number = {1,{magnitude,sizeof(magnitude)}};
    CONDITION(Borrowed) condition = {1}; FAILURE(Borrowed) failure = {1,{bytes,sizeof(bytes)}};
    LIST(Borrowed) list = {items,2};
    ok(fixture_prepare(context,instance,&text,&number,&condition,&failure,&list));
    memset(bytes,0,sizeof(bytes)); memset(magnitude,0,sizeof(magnitude));
}
/** Retained snapshots outlive the instance/context; reads do not consume a saved position. */
static void it_reads_all_profiles_independently_of_inputs_context_and_continuation(void) {
    API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance);
    prepare(context,instance);
    Snapshots first = {0}, second = {0}; ok(read_all(context,instance,&first)); ok(read_all(context,instance,&second));
    assert(first.text != second.text && first.failure != second.failure && first.list != second.list);
    expected_values(&first); expected_values(&second);
    TEXT(Value) *aliased = NULL;
    API(ApiResult) result = EXPORTED(read)(context,instance,&aliased,NULL,NULL,NULL,NULL,&aliased);
    assert(result.code == 1 && aliased == NULL); assert(API(result_release)(&result) == 0);
    result = read_all(context,instance,&first); assert(result.code == 1); assert(API(result_release)(&result) == 0);
    fixture_busy_child(context,instance,1);
    Snapshots denied = {0}; result = read_all(context,instance,&denied);
    assert(result.code == 2 && denied.text == NULL && denied.failure == NULL && denied.copy == NULL);
    assert(API(result_release)(&result) == 0);
    ok(EXPORTED(read)(context,instance,NULL,NULL,NULL,&denied.failure,NULL,NULL));
    release(&denied); fixture_busy_child(context,instance,0);
    FAILURE(Borrowed) no_failure = {0};
    ok(fixture_prepare(context,instance,NULL,NULL,NULL,&no_failure,NULL));
    Snapshots empty = {0}; ok(read_all(context,instance,&empty));
    FAILURE(Borrowed) failure = {0}; ok(FAILURE(view)(empty.failure,&failure)); assert(failure.failed == 0 && failure.description.length == 0);
    LIST(Borrowed) list = {0}; ok(LIST(view)(empty.list,&list)); assert(list.length == 0);
    NUMBER(Borrowed) zero = {0}; ok(NUMBER(view)(empty.number,&zero)); assert(zero.negative == 0 && zero.magnitude.length == 1 && zero.magnitude.data[0] == 0);
    release(&empty);
    no_failure.failed = 1;
    ok(fixture_prepare(context,instance,NULL,NULL,NULL,&no_failure,NULL));
    ok(EXPORTED(read)(context,instance,NULL,NULL,NULL,&empty.failure,NULL,NULL));
    ok(FAILURE(view)(empty.failure,&failure)); assert(failure.failed == 1 && failure.description.length == 0); release(&empty);
    fixture_yield(context,instance);
    ok(EXPORTED(read)(context,instance,NULL,NULL,NULL,&empty.failure,NULL,NULL));
    fixture_check_yield(context,instance); release(&empty);
    ok(API(instance_dispose)(context,&instance)); ok(API(context_dispose)(&context));
    expected_values(&first); expected_values(&second); release(&first); release(&second);
    assert(live_allocations() == 0);
}
/** Shape rejection preserves the old results; malformed semantic contents fail only admitted contexts. */
static void it_separates_shape_rejection_from_typed_decoder_failure(void) {
    API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance); prepare(context,instance);
    CONDITION(Borrowed) bad_condition = {2};
    API(ApiResult) result = fixture_prepare(context,instance,NULL,NULL,&bad_condition,NULL,NULL);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    TEXT(Borrowed) bad_shape = {NULL,1};
    result = fixture_prepare(context,instance,&bad_shape,NULL,NULL,NULL,NULL);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    LIST(Borrowed) bad_list = {&bad_shape,SIZE_MAX};
    result = fixture_prepare(context,instance,NULL,NULL,NULL,NULL,&bad_list);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    bad_list.length = 1;
    result = fixture_prepare(context,instance,NULL,NULL,NULL,NULL,&bad_list);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    NUMBER(Borrowed) bad_sign = {2,{NULL,0}};
    result = fixture_prepare(context,instance,NULL,&bad_sign,NULL,NULL,NULL);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    FAILURE(Borrowed) bad_discriminator = {2,{NULL,0}};
    result = fixture_prepare(context,instance,NULL,NULL,NULL,&bad_discriminator,NULL);
    assert(result.code == 1); assert(API(result_release)(&result) == 0);
    Snapshots values = {0}; ok(read_all(context,instance,&values)); expected_values(&values); release(&values);
    static const uint8_t invalid[] = {192,128}; TEXT(Borrowed) malformed = {invalid,sizeof(invalid)};
    result = fixture_prepare(context,instance,&malformed,NULL,NULL,NULL,NULL);
    assert(result.code == 5 && result.diagnostic.code != 0); assert(API(result_release)(&result) == 0);
    result = read_all(context,instance,&values); assert(result.code != 0 && values.text == NULL && values.failure == NULL);
    assert(API(result_release)(&result) == 0);
    result = API(context_dispose)(&context); assert(result.code == 5); assert(API(result_release)(&result) == 0);
    assert(live_allocations() == 0);
    for (unsigned kind = 0; kind != 2; kind++) {
        instance = (API(Instance)){0}; create(&context,&instance);
        static const uint8_t leading[] = {0,1}, byte[] = {97};
        NUMBER(Borrowed) bad_number = {0,{leading,2}}; FAILURE(Borrowed) bad_failure = {0,{byte,1}};
        result = fixture_prepare(context,instance,NULL,kind == 0 ? &bad_number : NULL,NULL,kind == 1 ? &bad_failure : NULL,NULL);
        assert(result.code == 5); assert(API(result_release)(&result) == 0);
        result = API(context_dispose)(&context); assert(result.code == 5); assert(API(result_release)(&result) == 0);
        assert(live_allocations() == 0);
    }
}
/** Fail every measured read allocation; partial snapshots remain unpublished and fully consumed. */
static void it_unwinds_every_real_retained_read_allocation_failure(void) {
    size_t count = 0;
    for (size_t offset = 0; offset <= count; offset++) {
        API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance); prepare(context,instance);
        Snapshots values = {0}; select_failure(offset);
        API(ApiResult) result = read_all(context,instance,&values);
        if (offset == 0) {
            assert(result.code == 0); count = allocation_calls(); assert(count != 0);
            assert(API(result_release)(&result) == 0); release(&values);
        } else {
            assert(result.code == 5 && values.text == NULL && values.number == NULL && values.condition == NULL
                && values.failure == NULL && values.list == NULL && values.copy == NULL);
            assert(API(result_release)(&result) == 0);
        }
        select_failure(0);
        result = API(context_dispose)(&context);
        assert(result.code == (offset == 0 ? 0 : 5)); assert(API(result_release)(&result) == 0);
        assert(live_allocations() == 0);
    }
}
/** All scalar profiles work as list elements, including failure descriptions and canonical zero. */
static void it_transports_lists_of_every_supported_scalar_profile(void) {
    API(Context) context = {0}; API(Instance) instance = {0}; create(&context,&instance);
    uint8_t zero = 0, magnitude[] = {1,0,254}, message[] = {97,0,208,176};
    NUMBER(Borrowed) number_items[] = {{0,{&zero,1}},{1,{magnitude,sizeof(magnitude)}}};
    CONDITION(Borrowed) condition_items[] = {{0},{1}};
    FAILURE(Borrowed) failure_items[] = {{0,{NULL,0}},{1,{message,sizeof(message)}}};
    NUMBERS(Borrowed) numbers = {number_items,2}; CONDITIONS(Borrowed) conditions = {condition_items,2};
    FAILURES(Borrowed) failures = {failure_items,2};
    NUMBERS(Value) *number_owner = NULL, *number_copy = NULL;
    CONDITIONS(Value) *condition_owner = NULL, *condition_copy = NULL;
    FAILURES(Value) *failure_owner = NULL, *failure_copy = NULL;
    fixture_other_lists(context,instance,&numbers,&conditions,&failures,&number_owner,&condition_owner,&failure_owner);
    memset(magnitude,0,sizeof(magnitude)); memset(message,0,sizeof(message));
    ok(NUMBERS(clone)(number_owner,&number_copy)); ok(CONDITIONS(clone)(condition_owner,&condition_copy));
    ok(FAILURES(clone)(failure_owner,&failure_copy));
    ok(API(context_dispose)(&context));
    ok(NUMBERS(release)(&number_owner)); ok(CONDITIONS(release)(&condition_owner)); ok(FAILURES(release)(&failure_owner));
    NUMBERS(Borrowed) number_values = {0}; CONDITIONS(Borrowed) condition_values = {0}; FAILURES(Borrowed) failure_values = {0};
    ok(NUMBERS(view)(number_copy,&number_values)); ok(CONDITIONS(view)(condition_copy,&condition_values));
    ok(FAILURES(view)(failure_copy,&failure_values));
    assert(number_values.length == 2 && number_values.items[0].negative == 0 && number_values.items[0].magnitude.length == 1 && number_values.items[0].magnitude.data[0] == 0);
    static const uint8_t expected_number[] = {1,0,254}, expected_message[] = {97,0,208,176};
    assert(number_values.items[1].negative == 1 && number_values.items[1].magnitude.length == sizeof(expected_number));
    assert(memcmp(number_values.items[1].magnitude.data,expected_number,sizeof(expected_number)) == 0);
    assert(condition_values.length == 2 && condition_values.items[0].value == 0 && condition_values.items[1].value == 1);
    assert(failure_values.length == 2 && failure_values.items[0].failed == 0 && failure_values.items[0].description.length == 0);
    assert(failure_values.items[1].failed == 1 && failure_values.items[1].description.length == sizeof(expected_message));
    assert(memcmp(failure_values.items[1].description.data,expected_message,sizeof(expected_message)) == 0);
    ok(NUMBERS(release)(&number_copy)); ok(CONDITIONS(release)(&condition_copy)); ok(FAILURES(release)(&failure_copy));
    assert(live_allocations() == 0);
}
/** Run actual public C calls while test probes isolate input preparation from future label dispatch. */
void run(void) {
    it_reads_all_profiles_independently_of_inputs_context_and_continuation();
    it_separates_shape_rejection_from_typed_decoder_failure();
    it_unwinds_every_real_retained_read_allocation_failure();
    it_transports_lists_of_every_supported_scalar_profile();
}
/** Exit only after both native context resources and independent public owners were consumed. */
int main(void) { run(); return 0; }
