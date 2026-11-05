#include "ama_native_sys_math_div.h"

void ama_native_sys_math_div(
    size_t parameter_left,
    size_t parameter_right,
    size_t* result_quot,
    size_t* result_error
) {
    if (parameter_right == 0) {
        *result_quot = 0;
        *result_error = 1;
        return;
    }
    *result_quot = parameter_left / parameter_right;
    *result_error = 0;
}
