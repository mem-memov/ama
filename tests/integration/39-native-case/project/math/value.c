#include "ama_native_math_value.h"

static size_t private_value(void) {
    return 1;
}

void ama_native_math_value(size_t* result_answer) {
    *result_answer = private_value();
}
