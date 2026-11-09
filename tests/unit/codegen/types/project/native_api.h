#ifndef AMA_TEST_NATIVE_API_H
#define AMA_TEST_NATIVE_API_H

#include "runtime/value.h"

typedef struct {
    unsigned char* bytes;
    size_t length;
    const void* home;
} TestBuffer;

extern size_t live_resources;
extern size_t literal_attempts;
extern size_t fail_literal_at;
extern size_t copy_attempts;
extern size_t fail_copy_at;

#endif
