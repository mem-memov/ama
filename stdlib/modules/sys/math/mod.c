#include "ama_native_d557581f729d6913.h"

void ama_native_d557581f729d6913(
    size_t parameter_left,
    size_t parameter_right,
    size_t* result_rem,
    size_t* result_error
) {
    if (parameter_right == 0) {
        *result_rem = 0;
        *result_error = 1;
        return;
    }
    *result_rem = parameter_left % parameter_right;
    *result_error = 0;
}
