#ifndef AMA_TESTS_COMMON_C
#define AMA_TESTS_COMMON_C
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_count;

static inline FILE* text_file(const char* text) {
    FILE* file = tmpfile();
    assert(file != NULL);
    size_t length = strlen(text);
    size_t written_bytes = fwrite(text, 1, length, file);
    assert(written_bytes == length);
    rewind(file);
    return file;
}

static inline void close_file(FILE* file) {
    int status = fclose(file);
    assert(status == 0);
}

static inline void assert_stream_equals(FILE* file, const char* text) {
    rewind(file);
    size_t offset = 0;
    for (;;) {
        int actual = fgetc(file);
        int expected = text[offset] ? (unsigned char)text[offset] : EOF;
        if (actual != expected)
            fprintf(stderr, "text differs at byte %zu: expected %d, got %d\n",
                offset, expected, actual);
        assert(actual == expected);
        if (actual == EOF) break;
        offset++;
    }
    assert(!ferror(file));
}
#endif
