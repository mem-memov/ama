/** @file
 * @brief Bind actual text components to counted allocation and reduced share limits.
 */
#ifndef AMA_TEXT_VALUE_TEST_COMMON
#define AMA_TEXT_VALUE_TEST_COMMON
#include "../../source/common.c"
#include "runtime/objects/objects.h"
#include <string.h>
#define AMA_NATIVE(name) test_text_##name
/** Supply the same nominal State alias as the generated native interface. */
typedef struct test_text_State test_text_State;
#include "../../../../stdlib/modules/text.h"
#define AMA_TEXT_ALLOCATE(size) test_allocate(NULL, size)
#define AMA_TEXT_RELEASE(pointer) test_release(NULL, pointer)
#define AMA_TEXT_SHARE_LIMIT 3
#include "../../../../stdlib/modules/text+/native/unicode.c"
#include "../../../../stdlib/modules/text+/native/storage.c"
#include "../../../../stdlib/modules/text+/native/values.c"
#undef AMA_TEXT_ALLOCATE
#undef AMA_TEXT_RELEASE
#undef AMA_TEXT_SHARE_LIMIT
#undef AMA_NATIVE

/** Require a native/runtime operation before observing its value effects. */
static void checked_text(AmaStatus status) { assert(status.code == AMA_OK); }
/** Construct one counted native literal through the production protocol. */
static test_text_State text_value(const char *value) {
    test_text_State result;
    checked_text(test_text_from_literal(&result, (AmaByteView){(const unsigned char *)value, strlen(value)}));
    return result;
}
/** Inspect the public borrowed-byte contract without relying on a terminator. */
static void expect_text(const test_text_State *state, const char *expected) {
    AmaByteView view;
    checked_text(test_text_view(state, &view));
    size_t length = strlen(expected);
    assert(view.length == length && (length == 0 || memcmp(view.data, expected, length) == 0));
}
#endif
