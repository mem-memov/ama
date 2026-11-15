/** @file
 * @brief Consume a compiled Ama component from a larger C program without its compiler SDK.
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) separates
 * explicit factories and execution; [EMBED-04](../../../spec/implementation/c-library.md#embed-04)
 * makes a retained text snapshot independent of the disposed context.
 */
#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "child-api.h"
#include "observer.h"

/** Every API result transfers any diagnostic owner, even when the operation succeeds. */
static void ok(CHILD(ApiResult) result) {
    assert(result.code == 0);
    uint32_t released = CHILD(result_release)(&result);
    assert(released == 0);
    example_returned();
}

/** Explicitly drive two yields without assuming that a yield schedules another call. */
int main(void) {
    example_observe_start();
    CHILD(Context) context = {0};
    CHILD(Instance) instance = {0};
    CHILD_TEXT(Value) *text = NULL, *retained = NULL;
    CHILD_CONDITION(Value) *previous = NULL;
    ok(CHILD(context_create)(&context));
    ok(CHILD_EXPORT(create)(context, &instance));
    ok(CHILD_EXPORT(read)(context, instance, &text, &previous));
    CHILD_TEXT(Borrowed) view = {0};
    ok(CHILD_TEXT(view)(text, &view));
    assert(view.length == 0);
    ok(CHILD_TEXT(release)(&text));
    ok(CHILD_CONDITION(release)(&previous));

    const uint8_t message[] = "standalone";
    CHILD_TEXT(Borrowed) input = {message, sizeof(message) - 1};
    uint32_t outcome = 0;
    ok(CHILD_EXPORT(entry)(context, instance, &input, &text, &previous, &outcome));
    retained = text;
    text = NULL;
    size_t yields = 0;
    while (outcome == CHILD_YIELDED) {
        yields++;
        ok(CHILD_CONDITION(release)(&previous));
        ok(CHILD_TEXT(release)(&text));
        ok(CHILD_EXPORT(resume)(context, instance, &text, &previous, &outcome));
    }
    assert(outcome == 1 && yields == 2);
    ok(CHILD_TEXT(release)(&text));
    ok(CHILD_CONDITION(release)(&previous));
    ok(CHILD(context_dispose)(&context));
    assert(CHILD(code_can_unload)() == 2);
    ok(CHILD_TEXT(view)(retained, &view));
    assert(view.length == sizeof(message) - 1 && memcmp(view.data, message, view.length) == 0);
    ok(CHILD_TEXT(release)(&retained));
    assert(CHILD(code_can_unload)() == 0);
    example_observe_finish();
    puts("standalone: 2 yields, independent retained value");
    return 0;
}
