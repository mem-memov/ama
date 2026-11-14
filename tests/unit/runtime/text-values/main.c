/** @file
 * @brief Verify independent text values, UTF-8 boundaries and exact resource credits.
 */
#include "common.c"
#include "forest.c"

/** Either destruction order keeps the survivor's bytes and finalizes once. */
static void it_shares_immutable_bytes_and_preserves_both_destruction_orders(void) {
    for (size_t order = 0; order < 2; order++) {
        setup_test();
        test_text_State source = text_value("message"), copy;
        checked_text(test_text_clone(&copy, &source));
        assert(source.backing == copy.backing && source.backing->shares == 2 && live_allocations == 1);
        checked_text(test_text_destroy(order == 0 ? &source : &copy));
        expect_text(order == 0 ? &copy : &source, "message");
        checked_text(test_text_destroy(order == 0 ? &copy : &source));
        assert(live_allocations == 0);
        destroy_test();
    }
}

/** Replacement prepares before releasing aliases and never edits another copy. */
static void it_replaces_values_independently_including_aliased_source_bytes(void) {
    setup_test();
    test_text_State source = text_value("before"), copy;
    checked_text(test_text_clone(&copy, &source));
    AmaByteView aliased;
    checked_text(test_text_view(&source, &aliased));
    checked_text(test_text_replace(&source, aliased));
    assert(source.backing != copy.backing);
    checked_text(test_text_replace(&source, (AmaByteView){(const unsigned char *)"after", 5}));
    expect_text(&source, "after");
    expect_text(&copy, "before");
    checked_text(test_text_destroy(&source));
    checked_text(test_text_destroy(&copy));
    destroy_test();
}

/** Failed preparation leaves both old shares and bytes exactly unchanged. */
static void it_preserves_old_values_after_allocation_and_utf8_replacement_failure(void) {
    setup_test();
    test_text_State source = text_value("old"), copy;
    checked_text(test_text_clone(&copy, &source));
    failure_at = allocation_count + 1;
    AmaStatus status = test_text_replace(&source, (AmaByteView){(const unsigned char *)"new", 3});
    assert(status.code == AMA_OUT_OF_MEMORY && source.backing == copy.backing && source.backing->shares == 2);
    failure_at = 0;
    const unsigned char invalid[] = {0xed, 0xa0, 0x80};
    status = test_text_replace(&source, (AmaByteView){invalid, sizeof(invalid)});
    assert(status.code == AMA_INVALID_LITERAL && source.backing == copy.backing && source.backing->shares == 2);
    expect_text(&source, "old");
    checked_text(test_text_destroy(&copy));
    checked_text(test_text_destroy(&source));
    destroy_test();
}

/** Refuse before overflowing a reduced count; finalized credits become reusable. */
static void it_refuses_nonwrapping_share_exhaustion_without_publishing_a_destination(void) {
    setup_test();
    test_text_State source = text_value("value"), first, second, failed;
    checked_text(test_text_clone(&first, &source));
    checked_text(test_text_clone(&second, &source));
    AmaStatus status = test_text_clone(&failed, &source);
    assert(status.code == AMA_LIMIT_EXCEEDED && !failed.live && failed.backing == NULL
        && source.backing->shares == 3);
    checked_text(test_text_destroy(&first));
    checked_text(test_text_clone(&failed, &source));
    checked_text(test_text_destroy(&failed));
    checked_text(test_text_destroy(&second));
    checked_text(test_text_destroy(&source));
    status = test_text_destroy(&source);
    assert(status.code == AMA_INVALID_STATE && live_allocations == 0);
    destroy_test();
}

/** Exact scalar UTF-8 grammar includes NUL and all boundary code points. */
static void it_accepts_utf8_scalar_boundaries_and_rejects_malformed_encodings(void) {
    setup_test();
    const unsigned char valid[] = {0, 0x7f, 0xc2, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80,
        0xed, 0x9f, 0xbf, 0xee, 0x80, 0x80, 0xef, 0xbf, 0xbf,
        0xf0, 0x90, 0x80, 0x80, 0xf4, 0x8f, 0xbf, 0xbf};
    test_text_State value;
    checked_text(test_text_from_literal(&value, (AmaByteView){valid, sizeof(valid)}));
    AmaByteView view;
    checked_text(test_text_view(&value, &view));
    assert(view.length == sizeof(valid) && memcmp(view.data, valid, sizeof(valid)) == 0);
    checked_text(test_text_destroy(&value));
    const unsigned char invalid[][4] = {{0x80}, {0xc0, 0x80}, {0xe0, 0x9f, 0xbf},
        {0xed, 0xa0, 0x80}, {0xf0, 0x8f, 0xbf, 0xbf}, {0xf4, 0x90, 0x80, 0x80}, {0xf5, 0x80, 0x80, 0x80}, {0xe2, 0x28, 0xa1}};
    const size_t lengths[] = {1, 2, 3, 3, 4, 4, 4, 3};
    for (size_t index = 0; index < sizeof(lengths) / sizeof(*lengths); index++) {
        AmaStatus status = test_text_from_literal(&value, (AmaByteView){invalid[index], lengths[index]});
        assert(status.code == AMA_INVALID_LITERAL && !value.live && value.backing == NULL && live_allocations == 0);
    }
    for (size_t length = 1; length < 4; length++) {
        const unsigned char truncated[] = {0xf0, 0x90, 0x80, 0x80};
        AmaStatus status = test_text_from_literal(&value, (AmaByteView){truncated, length});
        assert(status.code == AMA_INVALID_LITERAL && !value.live && live_allocations == 0);
    }
    destroy_test();
}

/** Empty values and copies own no byte allocation; truth still inspects length. */
static void it_constructs_and_clears_empty_text_without_allocating(void) {
    setup_test();
    test_text_State empty, copy;
    size_t before = allocation_count;
    checked_text(test_text_initialize(&empty));
    checked_text(test_text_clone(&copy, &empty));
    bool condition = true;
    checked_text(test_text_to_condition(&copy, &condition));
    assert(!condition && allocation_count == before && empty.backing == NULL && copy.backing == NULL);
    checked_text(test_text_replace(&empty, (AmaByteView){0}));
    checked_text(test_text_destroy(&copy));
    checked_text(test_text_destroy(&empty));
    destroy_test();
}

/** Run production native algorithms and real runtime clone-prefix cleanup. */
void run(void) {
    it_shares_immutable_bytes_and_preserves_both_destruction_orders();
    it_replaces_values_independently_including_aliased_source_bytes();
    it_preserves_old_values_after_allocation_and_utf8_replacement_failure();
    it_refuses_nonwrapping_share_exhaustion_without_publishing_a_destination();
    it_accepts_utf8_scalar_boundaries_and_rejects_malformed_encodings();
    it_constructs_and_clears_empty_text_without_allocating();
    it_rolls_back_earlier_shares_after_a_later_owned_child_clone_fails();
}

/** Return after every credit, byte sequence and failure invariant has matched. */
int main(void) { run(); return EXIT_SUCCESS; }
