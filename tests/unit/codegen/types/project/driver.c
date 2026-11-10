#include "types.h"
#include "types.h" /* The generated header must support repeated inclusion. */
#include "bindings.h"
#include <assert.h>
#include <string.h>

const AmaType* second_descriptor(void);

int main(void) {
    RootState original, copy;
    AmaValue value = {&root_descriptor, &original};
    AmaValue copied = {&root_descriptor, &copy};
    assert(second_descriptor() == &root_descriptor);
    assert(ama_value_initialize(value) == AMA_STATUS_OK);
    assert(live_resources == 5);
    assert(original.root_first.length == 5 && memcmp(original.root_first.bytes, "first", 5) == 0);
    assert(original.root_copied.bytes != original.root_first.bytes);
    assert(memcmp(original.root_copied.bytes, "first", 5) == 0);
    assert(original.root_unicode.unicode_data.length == 0);
    assert(ama_value_initialize_copy(copied, value) == AMA_STATUS_OK && live_resources == 10);
    original.root_first.bytes[0] = 'x';
    assert(copy.root_first.bytes[0] == 'f');
    fail_copy_at = copy_attempts + 3;
    unsigned char* previous = copy.root_first.bytes;
    assert(ama_value_replace(copied, value) == AMA_STATUS_COPY_UNSUPPORTED);
    assert(copy.root_first.bytes == previous && live_resources == 10);
    assert(ama_value_replace(copied, value) == AMA_STATUS_OK);
    assert(copy.root_first.home == &copy.root_first);
    assert(copy.root_left.left_data.home == &copy.root_left.left_data);
    assert(copy.root_first.bytes[0] == 'x' && live_resources == 10);
    ama_value_destroy(copied);
    ama_value_destroy(value);
    assert(live_resources == 0);
    TestBuffer native_state, native_result;
    NativeFrame native_frame;
    AmaValue retained = {&native_frame_type, &native_frame};
    assert(ama_value_initialize((AmaValue){&native_descriptor, &native_state}) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&native_descriptor, &native_result}) == AMA_STATUS_OK);
    assert(ama_value_initialize(retained) == AMA_STATUS_OK);
    assert(invoke_native(&native_state, &native_frame, NULL, 0) == AMA_STATUS_OK);
    assert(ama_call_read(native_schema(), retained, 0,
        (AmaValue){&native_descriptor, &native_result}) == AMA_STATUS_OK);
    assert(native_result.length == 8 && memcmp(native_result.bytes, "argument", 8) == 0);
    ama_value_destroy(retained);
    ama_value_destroy((AmaValue){&native_descriptor, &native_result});
    ama_value_destroy((AmaValue){&native_descriptor, &native_state});
    assert(live_resources == 0);

    RootFrame frame;
    AmaValue frame_value = {&frame_descriptor, &frame};
    assert(ama_value_initialize(frame_value) == AMA_STATUS_OK);
    assert(live_resources == 7 && frame.frame_output.bytes != frame.frame_input.bytes);
    assert(frame.frame_output.length == 5 && memcmp(frame.frame_output.bytes, "input", 5) == 0);
    ama_value_destroy(frame_value);
    assert(live_resources == 0);

    /* Initialization must unwind all earlier fields and any nested children. */
    fail_literal_at = literal_attempts + 4;
    assert(ama_value_initialize(value) == AMA_STATUS_LITERAL_INVALID);
    assert(live_resources == 0);
    assert(ama_value_initialize(value) == AMA_STATUS_OK);
    fail_literal_at = literal_attempts + 4;
    previous = original.root_first.bytes;
    assert(ama_value_reset(value) == AMA_STATUS_LITERAL_INVALID);
    assert(original.root_first.bytes == previous && live_resources == 5);
    ama_value_destroy(value);
    assert(live_resources == 0);
    return 0;
}
