#ifndef AMA_TEST_DRIVER_COMMON_C
#define AMA_TEST_DRIVER_COMMON_C

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/driver.h"

static FILE *output;
static FILE *errors;

static void setup_test(void) {
    output = tmpfile();
    errors = tmpfile();
    assert(output != NULL);
    assert(errors != NULL);
}

static void destroy_test(void) {
    int output_result = fclose(output);
    int errors_result = fclose(errors);
    assert(output_result == 0);
    assert(errors_result == 0);
}

static void read_stream(FILE *stream, char *buffer, size_t capacity) {
    int seek_result = fseek(stream, 0, SEEK_SET);
    assert(seek_result == 0);
    size_t length = fread(buffer, 1, capacity - 1, stream);
    assert(!ferror(stream));
    assert(length < capacity - 1);
    buffer[length] = '\0';
}

#endif
