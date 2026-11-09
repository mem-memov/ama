#ifndef AMA_TESTS_RUNTIME_COMMON_C
#define AMA_TESTS_RUNTIME_COMMON_C

#include "../../common.c"
#include "runtime/record.h"

typedef struct {
    unsigned char* bytes;
    size_t length;
    const void* home;
} BufferState;

typedef struct {
    BufferState first;
    BufferState second;
} PairState;

typedef struct {
    PairState pair;
    BufferState last;
} TreeState;

static size_t live_allocations;
static size_t initialize_attempts;
static size_t copy_attempts;
static size_t fail_initialize_at;
static size_t fail_copy_at;
static char destruction_order[64];
static size_t destruction_count;

static AmaStatus buffer_literal(const AmaType* type, void* destination,
    const unsigned char* bytes, size_t length) {
    (void)type;
    unsigned char* owned = malloc(length ? length : 1);
    if (!owned) return AMA_STATUS_MEMORY;
    if (length) memcpy(owned, bytes, length);
    BufferState* state = destination;
    *state = (BufferState){owned, length, destination};
    live_allocations++;
    return AMA_STATUS_OK;
}

static AmaStatus buffer_initialize(const AmaType* type, void* destination) {
    initialize_attempts++;
    /* A failing native callback must clean its own partial acquisition. */
    if (initialize_attempts == fail_initialize_at) {
        void* partial = malloc(1);
        free(partial);
        return AMA_STATUS_MEMORY;
    }
    return buffer_literal(type, destination, (const unsigned char*)"default", 7);
}

static AmaStatus buffer_copy(const AmaType* type, void* destination, const void* source) {
    copy_attempts++;
    if (copy_attempts == fail_copy_at) return AMA_STATUS_MEMORY;
    const BufferState* state = source;
    assert(state->home == state);
    return buffer_literal(type, destination, state->bytes, state->length);
}

static void buffer_move(const AmaType* type, void* destination, void* source) {
    (void)type;
    BufferState* original = source;
    assert(original->home == original);
    BufferState* moved = destination;
    *moved = *original;
    moved->home = moved;
    *original = (BufferState){0};
}

static void buffer_destroy(const AmaType* type, void* state) {
    (void)type;
    BufferState* buffer = state;
    assert(buffer->home == buffer && buffer->bytes && live_allocations);
    assert(destruction_count < sizeof(destruction_order));
    destruction_order[destruction_count++] = buffer->length ? (char)buffer->bytes[0] : '_';
    free(buffer->bytes);
    live_allocations--;
    *buffer = (BufferState){0};
}

static const AmaType buffer_type = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = 1,
    .module_path = "test/buffer",
    .size = sizeof(BufferState),
    .alignment = _Alignof(BufferState),
    .initialize = buffer_initialize,
    .initialize_literal = buffer_literal,
    .initialize_copy = buffer_copy,
    .move = buffer_move,
    .destroy = buffer_destroy
};

static const AmaField pair_fields[] = {
    {offsetof(PairState, first), &buffer_type},
    {offsetof(PairState, second), &buffer_type}
};
static const AmaRecord pair_record = {pair_fields, 2};
static const AmaType pair_type = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = 2,
    .module_path = "test/pair",
    .size = sizeof(PairState),
    .alignment = _Alignof(PairState),
    .context = &pair_record,
    .initialize = ama_record_initialize,
    .initialize_copy = ama_record_initialize_copy,
    .move = ama_record_move,
    .destroy = ama_record_destroy
};

static const AmaField tree_fields[] = {
    {offsetof(TreeState, pair), &pair_type},
    {offsetof(TreeState, last), &buffer_type}
};
static const AmaRecord tree_record = {tree_fields, 2};
static const AmaType tree_type = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = 3,
    .module_path = "test/tree",
    .size = sizeof(TreeState),
    .alignment = _Alignof(TreeState),
    .context = &tree_record,
    .initialize = ama_record_initialize,
    .initialize_copy = ama_record_initialize_copy,
    .move = ama_record_move,
    .destroy = ama_record_destroy
};

static AmaStatus number_initialize(const AmaType* type, void* destination) {
    (void)type;
    *(uint32_t*)destination = 0;
    return AMA_STATUS_OK;
}

static AmaStatus number_literal(const AmaType* type, void* destination,
    const unsigned char* bytes, size_t length) {
    (void)type;
    if (!length) return AMA_STATUS_LITERAL_INVALID;
    uint32_t number = 0;
    for (size_t index = 0; index < length; index++) {
        if (bytes[index] < '0' || bytes[index] > '9') return AMA_STATUS_LITERAL_INVALID;
        unsigned int digit = bytes[index] - '0';
        if (number > (UINT32_MAX - digit) / 10) return AMA_STATUS_LITERAL_INVALID;
        number = number * 10 + digit;
    }
    *(uint32_t*)destination = number;
    return AMA_STATUS_OK;
}

static AmaStatus number_copy(const AmaType* type, void* destination, const void* source) {
    (void)type;
    *(uint32_t*)destination = *(const uint32_t*)source;
    return AMA_STATUS_OK;
}

static void number_move(const AmaType* type, void* destination, void* source) {
    (void)type;
    *(uint32_t*)destination = *(uint32_t*)source;
}

static void number_destroy(const AmaType* type, void* state) {
    (void)type;
    (void)state;
}

/* An illustrative native type, not a privileged compiler numeric category. */
static const AmaType number_type = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = 4,
    .module_path = "test/number",
    .size = sizeof(uint32_t),
    .alignment = _Alignof(uint32_t),
    .initialize = number_initialize,
    .initialize_literal = number_literal,
    .initialize_copy = number_copy,
    .move = number_move,
    .destroy = number_destroy
};

static void setup_test(void) {
    test_count++;
    assert(live_allocations == 0);
    initialize_attempts = 0;
    copy_attempts = 0;
    fail_initialize_at = 0;
    fail_copy_at = 0;
    destruction_count = 0;
    memset(destruction_order, 0, sizeof(destruction_order));
}

static void destroy_test(void) {
    assert(live_allocations == 0);
}

#endif
