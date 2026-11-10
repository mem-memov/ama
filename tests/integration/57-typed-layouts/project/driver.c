#include "ama_types.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MainState ama_state_43d7649607aeb385
#define MainFrame ama_frame_43d7649607aeb385
#define main_type ama_type_43d7649607aeb385
#define frame_type ama_frame_type_43d7649607aeb385
#define body_saved ama_field_ebfa62135e603750
#define body_copy ama_field_de448b5c9d74663d
#define body_item ama_field_1059c68c9f52c1ef
#define child_data ama_field_16624add7c2a598a
#define frame_input ama_field_dea4885357ec4817
#define frame_output ama_field_e218ab7fc9a99023

int main(void) {
    MainState state, copied;
    MainFrame frame;
    AmaValue value = {&main_type, &state};
    AmaValue copy = {&main_type, &copied};
    AmaValue interface = {&frame_type, &frame};
    assert(ama_value_initialize(value) == AMA_STATUS_OK && live_resources == 3);
    assert(state.body_saved.length == 15);
    assert(memcmp(state.body_saved.bytes, "hello # literal", 15) == 0);
    assert(state.body_saved.bytes != state.body_copy.bytes);
    assert(state.body_item.child_data.length == 5);
    assert(ama_value_initialize_copy(copy, value) == AMA_STATUS_OK && live_resources == 6);
    state.body_saved.bytes[0] = 'H';
    assert(copied.body_saved.bytes[0] == 'h');
    assert(ama_value_initialize(interface) == AMA_STATUS_OK && live_resources == 8);
    assert(frame.frame_input.bytes != frame.frame_output.bytes);
    assert(memcmp(frame.frame_output.bytes, "input", 5) == 0);
    ama_value_destroy(interface);
    ama_value_destroy(copy);
    ama_value_destroy(value);
    assert(live_resources == 0);
    puts("typed layouts: ok");
    return 0;
}
