#include "ama_native_sys_math_and.h"

void ama_native_sys_math_and(size_t parameter_a, size_t parameter_b, size_t* result_result) {
    *result_result = parameter_a && parameter_b;
}
