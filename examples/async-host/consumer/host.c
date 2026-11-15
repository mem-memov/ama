/** @file
 * @brief Drive actual unloaded/reloaded components through their SDK-free checked C API and a separately owned native backend.
 */
#include "backend.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

/** Consume every diagnostic owner, including otherwise successful API results. */
static void ok(API(ApiResult) result) {
    assert(result.code == 0 && result.diagnostic.code == 0);
    uint32_t released = API(result_release)(&result);
    assert(released == 0);
}
/** Construct one explicit root; library loading itself executes no Ama body. */
static void create(API(Context) *context, API(Instance) *instance) {
    ok(API(context_create)(context));
    ok(RUN(create)(*context, instance));
}
/** Submit without retaining the default Waiting output snapshot. */
static void enter(API(Context) context, API(Instance) instance, bool immediate, bool cancel) {
    FLAG(Borrowed) mode = {(uint8_t)immediate}, requested = {(uint8_t)cancel};
    uint32_t outcome = 99;
    ok(RUN(entry)(context, instance, &mode, &requested, NULL, &outcome));
    assert(outcome == 2);
}
/** Exactly one resume commits a terminal ordinary value; a further resume cannot publish another result. */
static TEXT(Value) *complete(API(Context) context, API(Instance) instance, const char *expected) {
    TEXT(Value) *message = NULL;
    uint32_t outcome = 99;
    ok(RUN(resume)(context, instance, &message, &outcome));
    assert(outcome == 1 && message != NULL);
    TEXT(Borrowed) view = {0};
    ok(TEXT(view)(message, &view));
    assert(view.length == strlen(expected) && memcmp(view.data, expected, view.length) == 0);
    TEXT(Value) *other = NULL;
    outcome = 77;
    API(ApiResult) refused = RUN(resume)(context, instance, &other, &outcome);
    assert(refused.code == 3 && outcome == 77 && other == NULL);
    uint32_t released = API(result_release)(&refused);
    assert(released == 0);
    return message;
}
/** Native and exported owners independently prevent unload after the context has disappeared. */
static void close_with_snapshot(API(Context) *context, TEXT(Value) **message) {
    ok(API(context_dispose)(context));
    uint32_t state = API(code_can_unload)();
    assert(state == 2);
    ok(TEXT(release)(message));
    state = API(code_can_unload)();
    assert(state == 0);
}

/** A native worker remains possible after context close; no manual caller pin compensates for a missing runtime lease. */
static void pending_disposal(void) {
    load_image();
    API(Context) context = {0}; API(Instance) instance = {0};
    create(&context, &instance);
    enter(context, instance, false, false);
    ok(API(context_dispose)(&context));
    uint32_t state = API(code_can_unload)();
    assert(state == 2);
    BackendResult result = backend_finish();
    assert(result == BACKEND_CLOSED);
    state = API(code_can_unload)();
    assert(state == 0);
    unload_image();
}
/** Both native-before-return and later worker completion are retained across the root's explicit yield. */
static void completion(bool immediate) {
    load_image();
    API(Context) context = {0}; API(Instance) instance = {0};
    create(&context, &instance);
    enter(context, instance, immediate, false);
    assert(backend_domain_changed());
    BackendResult result = backend_finish();
    assert(result == BACKEND_ACCEPTED);
    TEXT(Value) *message = complete(context, instance, "completed");
    close_with_snapshot(&context, &message);
    unload_image();
}
/** Arrival before cancellation commit is still cancellable; a late arrival after commit is obsolete. */
static void cancellation(bool immediate) {
    load_image();
    API(Context) context = {0}; API(Instance) instance = {0};
    create(&context, &instance);
    enter(context, instance, immediate, true);
    assert(backend_domain_changed());
    TEXT(Value) *message = complete(context, instance, "cancelled");
    BackendResult result = backend_finish();
    assert(result == (immediate ? BACKEND_ACCEPTED : BACKEND_OBSOLETE));
    close_with_snapshot(&context, &message);
    unload_image();
}
/** Restart closes the old service before the old callback fires, while a new native image lease remains valid. */
static void restart(void) {
    load_image();
    API(Context) context = {0}; API(Instance) instance = {0};
    create(&context, &instance);
    enter(context, instance, false, false);
    assert(backend_domain_changed());
    enter(context, instance, false, false);
    assert(!backend_domain_changed());
    BackendResult old = backend_finish();
    assert(old == BACKEND_CLOSED);
    BackendResult current = backend_finish();
    assert(current == BACKEND_ACCEPTED);
    TEXT(Value) *message = complete(context, instance, "completed");
    close_with_snapshot(&context, &message);
    unload_image();
}
/** A constructor failure after code acquisition leaves only the explicitly returned diagnostic owners. */
static void constructor_failure(void) {
    load_image();
    API(Context) context = {0}; API(Instance) instance = {0};
    ok(API(context_create)(&context));
    ok(INVALID(create)(context, &instance));
    uint32_t outcome = 77;
    API(ApiResult) failed = INVALID(entry)(context, instance, &outcome);
    assert(failed.code == 0 && outcome == 3 && failed.diagnostic.code != 0);
    API(ApiResult) disposed = API(context_dispose)(&context);
    assert(disposed.code == 5);
    uint32_t released = API(result_release)(&disposed);
    assert(released == 0);
    uint32_t state = API(code_can_unload)();
    assert(state == 2);
    released = API(result_release)(&failed);
    assert(released == 0);
    state = API(code_can_unload)();
    assert(state == 0);
    BackendResult no_job = backend_finish();
    assert(no_job == BACKEND_NONE);
    unload_image();
}
/** The host's join and single-caller loader barrier exclude all native creator-code frames before dlclose. */
int main(void) {
    pending_disposal();
    completion(false);
    completion(true);
    cancellation(false);
    cancellation(true);
    restart();
    constructor_failure();
    puts("asynchronous component lifetime accepted");
    return 0;
}
