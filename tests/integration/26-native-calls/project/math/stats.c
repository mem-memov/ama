#include "ama_native_6a0b51bffc7225f8.h"
#include <assert.h>

void ama_native_6a0b51bffc7225f8(
    size_t parameter_left,
    size_t parameter_right,
    size_t* result_sum,
    size_t* result_product,
    size_t* result_error
) {
    assert(result_sum && result_product && result_error);
    if (!parameter_right) {
        *result_error = 1;
        return;
    }
    *result_sum = parameter_left + parameter_right;
    *result_product = parameter_left * parameter_right;
    *result_error = 0;
    parameter_left = 99;
    (void)parameter_left;
}
