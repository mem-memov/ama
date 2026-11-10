#include "ama_types.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define EchoFrame ama_frame_e957b3907961d9c0
#define echo_frame_type ama_frame_type_e957b3907961d9c0
#define invoke_echo ama_invoke_e957b3907961d9c0
#define echo_schema ama_call_schema_e957b3907961d9c0
#define PingState ama_state_ad4c8e009dd42c75
#define PingFrame ama_frame_ad4c8e009dd42c75
#define ping_type ama_type_ad4c8e009dd42c75
#define ping_frame_type ama_frame_type_ad4c8e009dd42c75
#define invoke_ping ama_invoke_ad4c8e009dd42c75

int main(void) {
    EchoState state;
    EchoFrame retained;
    TestBuffer input, output;
    assert(ama_value_initialize((AmaValue){&echo_descriptor, &state}) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&echo_frame_type, &retained}) == AMA_STATUS_OK);
    assert(ama_value_initialize_literal((AmaValue){&native_descriptor, &input},
        (const unsigned char*)"hello", 5) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&native_descriptor, &output}) == AMA_STATUS_OK);
    AmaValue frame = {&echo_frame_type, &retained};
    AmaValue result = {&native_descriptor, &output};
    assert(ama_call_read(echo_schema(), frame, 0, result) == AMA_STATUS_OK);
    assert(state.executions == 0 && output.length == 6 && memcmp(output.bytes, "before", 6) == 0);
    AmaCallInput argument = {0, {&native_descriptor, &input}};
    assert(invoke_echo(&state, &retained, &argument, 1) == AMA_STATUS_OK && state.executions == 1);
    input.bytes[0] = 'x';
    assert(ama_call_read(echo_schema(), frame, 0, result) == AMA_STATUS_OK);
    assert(output.bytes[0] == 'h' && output.length == 5 && state.executions == 1);
    state.fail = true;
    assert(invoke_echo(&state, &retained, NULL, 0) == AMA_STATUS_LITERAL_INVALID);
    assert(ama_call_read(echo_schema(), frame, 0, result) == AMA_STATUS_OK);
    assert(state.executions == 2 && output.bytes[0] == 'h');
    state.fail = false;
    assert(invoke_echo(&state, &retained, NULL, 0) == AMA_STATUS_OK);
    assert(ama_call_read(echo_schema(), frame, 0, result) == AMA_STATUS_OK);
    assert(state.executions == 3 && output.length == 7 && memcmp(output.bytes, "default", 7) == 0);
    ama_value_destroy(result);
    ama_value_destroy((AmaValue){&native_descriptor, &input});
    ama_value_destroy(frame);
    ama_value_destroy((AmaValue){&echo_descriptor, &state});
    assert(live_resources == 0);
    PingState ping;
    PingFrame ping_frame;
    assert(ama_value_initialize((AmaValue){&ping_type, &ping}) == AMA_STATUS_OK);
    assert(ama_value_initialize((AmaValue){&ping_frame_type, &ping_frame}) == AMA_STATUS_OK);
    assert(invoke_ping(&ping, &ping_frame, NULL, 0) == AMA_STATUS_OK && ping == 1);
    ama_value_destroy((AmaValue){&ping_frame_type, &ping_frame});
    ama_value_destroy((AmaValue){&ping_type, &ping});
    puts("typed native calls: ok");
    return 0;
}
