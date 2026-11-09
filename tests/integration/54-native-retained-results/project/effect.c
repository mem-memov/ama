#include "ama_native_1dd250c9187980d7.h"
#include <assert.h>
#include <stdio.h>

static size_t executions;

void ama_native_1dd250c9187980d7(size_t parameter_step, size_t* result_value, size_t* result_marker) {
    assert(*result_value == 42 && *result_marker == 9);
    executions++;
    puts("executed");
    *result_value = 100 + executions;
    *result_marker = parameter_step;
}
