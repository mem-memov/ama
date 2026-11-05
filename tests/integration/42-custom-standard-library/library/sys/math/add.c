#include "ama_native_sys_math_add.h"

/* An intentionally different implementation proves which library was loaded. */
void ama_native_sys_math_add(size_t parameter_left, size_t parameter_right, size_t* result_sum) {
    *result_sum = parameter_left * parameter_right;
}
