#ifndef TEST_BUFFER_H
#define TEST_BUFFER_H
#include "runtime/value.h"

typedef struct {
    unsigned char* bytes;
    size_t length;
    const void* home;
} TestBuffer;

typedef TestBuffer ama_state_eb0bcc138445b561;
extern const AmaType ama_type_eb0bcc138445b561;
#define native_descriptor ama_type_eb0bcc138445b561
#define TEST_NATIVE_IDENTITY UINT64_C(0xeb0bcc138445b561)
extern size_t live_resources;
extern size_t literal_attempts;
extern size_t fail_literal_at;
extern size_t copy_attempts;
extern size_t fail_copy_at;
#endif
