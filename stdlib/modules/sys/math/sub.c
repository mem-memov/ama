#include "ama_native_sys_math_sub.h"

void ama_native_sys_math_sub(size_t parameter_left, size_t parameter_right, size_t* result_diff) {
    *result_diff = parameter_left - parameter_right;
}
