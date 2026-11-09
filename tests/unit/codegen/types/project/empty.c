#include "types.h"
#include "bindings.h"
#include <assert.h>

int main(void) {
    RootState state;
    RootFrame frame;
    AmaValue state_value = {&root_descriptor, &state};
    AmaValue frame_value = {&frame_descriptor, &frame};
    assert(sizeof(state) > 0 && sizeof(frame) > 0);
    assert(ama_value_initialize(state_value) == AMA_STATUS_OK);
    assert(ama_value_initialize(frame_value) == AMA_STATUS_OK);
    assert(ama_value_reset(state_value) == AMA_STATUS_OK);
    ama_value_destroy(frame_value);
    ama_value_destroy(state_value);
    return 0;
}
