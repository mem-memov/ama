#include "../common.c"

#include "native/status/index.h"
#include "native/status/library.h"

static void it_represents_success_without_allocating_a_message(void) {
    AmaStatus status = {0};
    assert(ama_status_is_success(status));
    assert(ama_status_has_valid_message_view(status));
    assert(status.message == NULL && status.message_length == 0);
}

static void it_preserves_unterminated_program_lifetime_message_bytes(void) {
    static const char message[] = {'o', 'o', 'm'};
    AmaStatus status = ama_status_make(AMA_OUT_OF_MEMORY, message, sizeof(message));
    assert(status.message == message && status.message_length == 3);
    assert(!ama_status_is_success(status));
    status.message = NULL;
    assert(!ama_status_has_valid_message_view(status));
}

static void it_classifies_index_results_by_code_and_never_message_text(void) {
    AmaStatus refused = ama_status_make(AMA_INVALID_INDEX, NULL, 0);
    AmaStatus failed = ama_status_make(AMA_OUT_OF_MEMORY, "invalid index", 13);
    assert(ama_index_outcome(ama_status_success()) == AMA_INDEX_CONVERTED);
    assert(ama_index_outcome(refused) == AMA_INDEX_REFUSED);
    assert(ama_index_outcome(failed) == AMA_INDEX_FAILED);
}

static void it_assigns_collision_free_library_codes_and_rejects_zero_fields(void) {
    for (uint16_t namespace_id = 1; namespace_id <= 8; namespace_id++) {
        for (uint16_t reason = 1; reason <= 8; reason++) {
            uint32_t code = 0;
            AmaStatus status = ama_library_status_code(namespace_id, reason, &code);
            assert(ama_status_is_success(status));
            assert((code >> 16) == namespace_id && (code & UINT32_C(65535)) == reason);
        }
    }
    uint32_t code = 77;
    AmaStatus status = ama_library_status_code(0, 1, &code);
    assert(status.code == AMA_INVALID_STATE && code == 77);
    status = ama_library_status_code(1, 0, &code);
    assert(status.code == AMA_INVALID_STATE && code == 77);
    status = ama_library_status_code(UINT16_MAX, UINT16_MAX, &code);
    assert(ama_status_is_success(status) && code == UINT32_MAX);
}

void run(void) {
    it_represents_success_without_allocating_a_message();
    it_preserves_unterminated_program_lifetime_message_bytes();
    it_classifies_index_results_by_code_and_never_message_text();
    it_assigns_collision_free_library_codes_and_rejects_zero_fields();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
