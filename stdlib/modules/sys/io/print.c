#include "ama_native_sys_io_print.h"
#include <stdio.h>

void ama_native_sys_io_print(size_t parameter_value, size_t* result_error) {
    printf("%zu\n", parameter_value);
    *result_error = 0;
}
