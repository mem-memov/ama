/** @file
 * @brief Drive a native-owned child from outer Ama and retain a copied failure after cleanup.
 * [EMBED-05](../../../spec/implementation/c-library.md#embed-05) permits explicit
 * supervision, while [EMBED-08](../../../spec/implementation/c-library.md#embed-08)
 * keeps child C return separate from the parent's explicit continuation.
 */
#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "parent-api.h"
#include "child-api.h"
#include "observer.h"

/** Independently released C transport owners; none contains an Ama field pointer. */
typedef struct {
    PARENT_TEXT(Value) *text;
    PARENT_CONDITION(Value) *previous, *waiting;
    PARENT_FAILURE(Value) *failure;
} Reply;

/** Consume the result diagnostic at the same boundary as its execution status. */
static void ok(PARENT(ApiResult) result) {
    assert(result.code == 0);
    uint32_t released = PARENT(result_release)(&result);
    assert(released == 0);
    example_returned();
}

/** Release each owner with its originating component rather than the caller allocator. */
static void release(Reply *reply) {
    ok(PARENT_TEXT(release)(&reply->text));
    ok(PARENT_CONDITION(release)(&reply->previous));
    ok(PARENT_CONDITION(release)(&reply->waiting));
    ok(PARENT_FAILURE(release)(&reply->failure));
}

/** Supply explicit supervision; reset is a request, never inferred from new text. */
static void enter(PARENT(Context) context, PARENT(Instance) instance,
    const char *message, bool reset, Reply *reply, uint32_t *outcome) {
    PARENT_TEXT(Borrowed) input = {(const uint8_t *)message, strlen(message)};
    PARENT_CONDITION(Borrowed) restarting = {(uint8_t)reset}, propagate = {0};
    ok(PARENT_EXPORT(entry)(context, instance, &input, &restarting, &propagate,
        &reply->text, &reply->previous, &reply->waiting, &reply->failure, outcome));
}

/** Resume takes no fresh input; every native C activation has already returned. */
static size_t complete(PARENT(Context) context, PARENT(Instance) instance,
    Reply *reply, uint32_t *outcome) {
    size_t yields = 0;
    while (*outcome == PARENT_YIELDED) {
        yields++;
        release(reply);
        ok(PARENT_EXPORT(resume)(context, instance, &reply->text, &reply->previous,
            &reply->waiting, &reply->failure, outcome));
    }
    assert(*outcome == 1);
    return yields;
}

/** The child can fail independently, be explicitly recreated, and complete again. */
int main(void) {
    example_observe_start();
    PARENT(Context) context = {0};
    PARENT(Instance) instance = {0};
    Reply reply = {0};
    uint32_t outcome = 0;
    ok(PARENT(context_create)(&context));
    ok(PARENT_EXPORT(create)(context, &instance));
    enter(context, instance, "nested", false, &reply, &outcome);
    size_t yields = complete(context, instance, &reply, &outcome);
    assert(yields == 2);
    release(&reply);

    enter(context, instance, "fault-cleanup", false, &reply, &outcome);
    assert(outcome == 1);
    PARENT_FAILURE(Value) *report = reply.failure;
    reply.failure = NULL;
    release(&reply);
    PARENT_FAILURE(Borrowed) error = {0};
    ok(PARENT_FAILURE(view)(report, &error));
    const char expected[] = "Deliberate child execution failure";
    assert(error.failed == 1 && error.description.length == sizeof(expected) - 1);
    assert(memcmp(error.description.data, expected, sizeof(expected) - 1) == 0);
    assert(CHILD(code_can_unload)() == 0);

    enter(context, instance, "recovered", true, &reply, &outcome);
    yields = complete(context, instance, &reply, &outcome);
    assert(yields == 2);
    ok(PARENT_FAILURE(view)(reply.failure, &error));
    assert(error.failed == 0 && error.description.length == 0);
    release(&reply);
    ok(PARENT(context_dispose)(&context));
    assert(CHILD(code_can_unload)() == 0 && PARENT(code_can_unload)() == 2);
    ok(PARENT_FAILURE(view)(report, &error));
    assert(error.failed == 1 && error.description.length == sizeof(expected) - 1);
    assert(memcmp(error.description.data, expected, sizeof(expected) - 1) == 0);
    ok(PARENT_FAILURE(release)(&report));
    assert(PARENT(code_can_unload)() == 0);
    example_observe_finish();
    puts("nested: explicit yields, supervised failure, explicit reset");
    return 0;
}
