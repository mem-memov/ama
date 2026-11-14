/** @file
 * @brief Exercise entry/resume/read exports with no schemas, arguments or result slots.
 */
#include "consumer-api.h"
/* Assertions belong to this test consumer even when its component is built with NDEBUG. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
/** Require API success and release its initialized diagnostic owner. */
static void ok(API(ApiResult) result) {
    assert(result.code == 0);
    uint32_t released = API(result_release)(&result); assert(released == 0);
}
/** Zero-field roots can yield, resume, restart and dispose without manufacturing any value. */
static void it_executes_an_empty_transport_interface_without_zero_length_c_arrays(void) {
    API(Context) context = {0}; API(Instance) instance = {0};
    ok(API(context_create)(&context)); ok(EXPORTED(create)(context,&instance));
    uint32_t outcome = 99; API(ApiResult) result = EXPORTED(resume)(context,instance,&outcome);
    assert(result.code == 3 && outcome == 99); assert(API(result_release)(&result) == 0);
    ok(EXPORTED(entry)(context,instance,&outcome)); assert(outcome == 2);
    ok(EXPORTED(read)(context,instance));
    ok(EXPORTED(resume)(context,instance,&outcome)); assert(outcome == 1);
    ok(EXPORTED(entry)(context,instance,&outcome)); assert(outcome == 2);
    ok(API(instance_dispose)(context,&instance)); ok(API(context_dispose)(&context));
    assert(live_allocations() == 0);
}
/** Register the actual zero-schema public API calls. */
void run(void) { it_executes_an_empty_transport_interface_without_zero_length_c_arrays(); }
/** Return after the empty root/context and all diagnostic owners have been consumed. */
int main(void) { run(); return 0; }
