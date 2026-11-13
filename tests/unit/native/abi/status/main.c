/**
 * @file
 * @brief Check status/message/library code validity and distinct index outcomes.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

#include "native/status/index.h"
#include "native/status/library.h"

/**
 * Assert that the component represents success without allocating a message. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_represents_success_without_allocating_a_message(void) {
    AmaStatus status = {0};
    assert(ama_status_is_success(status));
    assert(ama_status_has_valid_message_view(status));
    assert(status.message == NULL && status.message_length == 0);
}

/**
 * Assert that the component preserves unterminated program lifetime message bytes. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_preserves_unterminated_program_lifetime_message_bytes(void) {
    static const char message[] = {'o', 'o', 'm'};
    AmaStatus status = ama_status_make(AMA_OUT_OF_MEMORY, message, sizeof(message));
    assert(status.message == message && status.message_length == 3);
    assert(!ama_status_is_success(status));
    status.message = NULL;
    assert(!ama_status_has_valid_message_view(status));
}

/**
 * Assert that the component classifies index results by code and never message text. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_classifies_index_results_by_code_and_never_message_text(void) {
    AmaStatus refused = ama_status_make(AMA_INVALID_INDEX, NULL, 0);
    AmaStatus failed = ama_status_make(AMA_OUT_OF_MEMORY, "invalid index", 13);
    assert(ama_index_outcome(ama_status_success()) == AMA_INDEX_CONVERTED);
    assert(ama_index_outcome(refused) == AMA_INDEX_REFUSED);
    assert(ama_index_outcome(failed) == AMA_INDEX_FAILED);
}

/**
 * Assert that the component assigns collision free library codes and rejects zero fields. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
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

/** Check the complete allocated core domain and representative provider boundaries.
 * Reserved runtime codes and zero provider/reason fields are malformed transport,
 * not new refusals or silently successful conversions.
 */
static void it_checks_status_code_allocation_without_interpreting_message_text(void) {
    static const char message[] = {'n', 'o', 't', ' ', 's', 'u', 'c', 'c', 'e', 's', 's'};
    for (uint32_t code = AMA_OK; code <= AMA_IO_FAILURE; code++) {
        AmaStatus original = ama_status_make(code, message, sizeof(message));
        AmaStatus checked = ama_status_checked(original);
        assert(checked.code == code && checked.message == message);
        assert(checked.message_length == sizeof(message));
    }
    const uint32_t valid[] = {UINT32_C(65537), UINT32_C(131071), UINT32_MAX};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        assert(ama_status_has_valid_code(valid[index]));
        assert(ama_status_checked(ama_status_make(valid[index], NULL, 0)).code == valid[index]);
    }
    const uint32_t invalid[] = {UINT32_C(14), UINT32_C(65535), UINT32_C(65536), UINT32_C(4294901760)};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        assert(!ama_status_has_valid_code(invalid[index]));
        AmaStatus checked = ama_status_checked(ama_status_make(invalid[index], message, sizeof(message)));
        assert(checked.code == AMA_INVALID_STATE && checked.message != message);
        assert(ama_status_has_valid_message_view(checked));
    }
}

/** Reject malformed success and refusal before consumers can trust a conversion output.
 * A bounded immutable empty message remains valid; no terminating NUL is required.
 */
static void it_normalizes_malformed_success_and_refusal_with_static_failure_storage(void) {
    static const char message[] = {'x'};
    const uint32_t codes[] = {AMA_OK, AMA_INVALID_INDEX, AMA_OUT_OF_MEMORY};
    const char *fallback = NULL;
    for (size_t index = 0; index < sizeof(codes) / sizeof(*codes); index++) {
        AmaStatus malformed = ama_status_make(codes[index], NULL, 1);
        AmaStatus checked = ama_status_checked(malformed);
        assert(checked.code == AMA_INVALID_STATE && checked.message_length != 0);
        if (fallback == NULL) { fallback = checked.message; }
        assert(checked.message == fallback);
        assert(ama_index_outcome(malformed) == AMA_INDEX_FAILED);
    }
    AmaStatus empty = ama_status_checked(ama_status_make(AMA_OK, message, 0));
    assert(empty.code == AMA_OK && empty.message == message && empty.message_length == 0);
    assert(ama_index_outcome(ama_status_make(AMA_INVALID_INDEX, message, 1)) == AMA_INDEX_REFUSED);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_represents_success_without_allocating_a_message();
    it_preserves_unterminated_program_lifetime_message_bytes();
    it_classifies_index_results_by_code_and_never_message_text();
    it_assigns_collision_free_library_codes_and_rejects_zero_fields();
    it_checks_status_code_allocation_without_interpreting_message_text();
    it_normalizes_malformed_success_and_refusal_with_static_failure_storage();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
