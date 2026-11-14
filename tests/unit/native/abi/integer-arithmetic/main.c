/** @file
 * @brief Compare production signed arithmetic with independent bounded/decimal oracles.
 */
#include "../integer/common.c"

/** Construct a fresh arbitrary-precision value using independent decimal parsing. */
static AMA_NATIVE(State) parsed_integer(const char *text) {
    AMA_NATIVE(State) result;
    AmaStatus status = AMA_NATIVE(from_literal)(&result, (AmaByteView){(const unsigned char *)text, strlen(text)});
    assert(status.code == AMA_OK);
    return result;
}
/** Require exact mathematical equality, not a host-size narrowing of the result. */
static void expect_integer(const AMA_NATIVE(State) *actual, const char *expected) {
    AMA_NATIVE(State) oracle = parsed_integer(expected);
    int ordering = 17;
    AmaStatus status = AMA_NATIVE(compare)(actual, &oracle, &ordering);
    assert(status.code == AMA_OK && ordering == 0);
    status = AMA_NATIVE(destroy)(&oracle);
    assert(status.code == AMA_OK);
}
/** Observe a sum and its immutable sources through an independent decimal oracle. */
static void check_sum(const char *first, const char *second, const char *expected) {
    AMA_NATIVE(State) left = parsed_integer(first), right = parsed_integer(second), result;
    AmaStatus status = AMA_NATIVE(initialize)(&result);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(add)(&result, &left, &right);
    assert(status.code == AMA_OK);
    expect_integer(&result, expected);
    expect_integer(&left, first);
    expect_integer(&right, second);
    status = AMA_NATIVE(destroy)(&result);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&right);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&left);
    assert(status.code == AMA_OK);
}
/** Small complete sign/order combinations and actual base-256 boundaries use
 * host arithmetic only in a bounded domain that cannot overflow long long.
 */
static void it_matches_bounded_signed_sums_and_order_at_every_selected_limb_boundary(void) {
    setup_integer();
    const long long boundaries[] = {-65537, -65536, -65535, -257, -256, -255, -1,
        0, 1, 255, 256, 257, 65535, 65536, 65537};
    for (size_t pass = 0; pass < 2; pass++) {
        size_t count = pass == 0 ? 33 : sizeof(boundaries) / sizeof(*boundaries);
        for (size_t first = 0; first < count; first++) {
            for (size_t second = 0; second < count; second++) {
                long long left = pass == 0 ? (long long)first - 16 : boundaries[first];
                long long right = pass == 0 ? (long long)second - 16 : boundaries[second];
                char a[40], b[40], expected[40];
                int length = snprintf(a, sizeof(a), "%lld", left);
                assert(length > 0 && (size_t)length < sizeof(a));
                length = snprintf(b, sizeof(b), "%lld", right);
                assert(length > 0 && (size_t)length < sizeof(b));
                length = snprintf(expected, sizeof(expected), "%lld", left + right);
                assert(length > 0 && (size_t)length < sizeof(expected));
                check_sum(a, b, expected);
                AMA_NATIVE(State) x = parsed_integer(a), y = parsed_integer(b);
                int ordering = 7;
                AmaStatus status = AMA_NATIVE(compare)(&x, &y, &ordering);
                assert(status.code == AMA_OK && ordering == (left < right ? -1 : left > right ? 1 : 0));
                status = AMA_NATIVE(destroy)(&x);
                assert(status.code == AMA_OK);
                status = AMA_NATIVE(destroy)(&y);
                assert(status.code == AMA_OK);
            }
        }
    }
    destroy_test();
}
/** Carries and cancellation extend beyond every built-in C integer range. */
static void it_handles_unbounded_carries_negative_sums_and_exact_cancellation(void) {
    setup_integer();
    check_sum("999999999999999999999999999999999999999", "1", "1000000000000000000000000000000000000000");
    check_sum("-999999999999999999999999999999999999999", "-1", "-1000000000000000000000000000000000000000");
    check_sum("999999999999999999999999999999999999999", "-999999999999999999999999999999999999999", "0");
    check_sum("-1", "999999999999999999999999999999999999999", "999999999999999999999999999999999999998");
    destroy_test();
}
/** Preparation reads both sources fully before consuming an aliased old result. */
static void it_supports_left_right_and_both_source_aliases(void) {
    setup_integer();
    AMA_NATIVE(State) left = parsed_integer("65535"), right = parsed_integer("1");
    AmaStatus status = AMA_NATIVE(add)(&left, &left, &right);
    assert(status.code == AMA_OK);
    expect_integer(&left, "65536");
    status = AMA_NATIVE(add)(&right, &left, &right);
    assert(status.code == AMA_OK);
    expect_integer(&right, "65537");
    status = AMA_NATIVE(add)(&left, &left, &left);
    assert(status.code == AMA_OK);
    expect_integer(&left, "131072");
    status = AMA_NATIVE(destroy)(&right);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&left);
    assert(status.code == AMA_OK);
    destroy_test();
}
/** No source or destination credit changes when the new magnitude cannot allocate. */
static void it_preserves_sources_and_the_old_result_after_preparation_failure(void) {
    setup_integer();
    AMA_NATIVE(State) left = parsed_integer("256"), right = parsed_integer("-1"), result = parsed_integer("9");
    failure_at = allocation_count + 1;
    AmaStatus status = AMA_NATIVE(add)(&result, &left, &right);
    assert(status.code == AMA_OUT_OF_MEMORY);
    failure_at = 0;
    expect_integer(&left, "256");
    expect_integer(&right, "-1");
    expect_integer(&result, "9");
    failure_at = allocation_count + 1;
    status = AMA_NATIVE(add)(&left, &left, &right);
    assert(status.code == AMA_OUT_OF_MEMORY);
    failure_at = 0;
    expect_integer(&left, "256");
    status = AMA_NATIVE(destroy)(&result);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&right);
    assert(status.code == AMA_OK);
    status = AMA_NATIVE(destroy)(&left);
    assert(status.code == AMA_OK);
    destroy_test();
}
/** Use public native operations over the actual maintained implementation. */
void run(void) {
    (void)roundtrip_size;
    it_matches_bounded_signed_sums_and_order_at_every_selected_limb_boundary();
    it_handles_unbounded_carries_negative_sums_and_exact_cancellation();
    it_supports_left_right_and_both_source_aliases();
    it_preserves_sources_and_the_old_result_after_preparation_failure();
}
/** Return after numerical, alias and failure-credit contracts match. */
int main(void) { run(); return EXIT_SUCCESS; }
