#include "ama_native_value.h"
#include <assert.h>

void ama_native_value(size_t* result_value) {
    assert(result_value && *result_value == 42);
    *result_value += 1;
}
