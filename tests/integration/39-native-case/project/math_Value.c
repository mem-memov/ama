#include "ama_native_math_Value.h"

static size_t private_value(void) {
    return 2;
}

void ama_native_math_Value(size_t* result_answer) {
    *result_answer = private_value();
}
