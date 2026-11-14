/** @file
 * @brief Compare signed magnitudes and prepare exact base-256 sums without narrowing.
 * [LIB-01](../../../../spec/language/library.md#lib-01) keeps arithmetic in the
 * library; [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * preserves an existing result until every new limb is prepared successfully.
 */

/** Canonical positive magnitudes order by limb count, then most significant
 * differing limb. Equal encodings denote equal mathematical magnitudes.
 */
static int AMA_NATIVE(compare_magnitude)(const AMA_NATIVE(State) *left, const AMA_NATIVE(State) *right) {
    if (left->count != right->count) { return left->count < right->count ? -1 : 1; }
    for (size_t index = left->count; index != 0; index--) {
        if (left->limbs[index - 1] != right->limbs[index - 1]) {
            return left->limbs[index - 1] < right->limbs[index - 1] ? -1 : 1;
        }
    }
    return 0;
}

/** Sign orders disjoint halves; negative values reverse magnitude order. */
AmaStatus AMA_NATIVE(compare)(const AMA_NATIVE(State) *left, const AMA_NATIVE(State) *right, int *output) {
    if (!left->live || !right->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    int result = left->negative != right->negative ? (left->negative ? -1 : 1)
        : AMA_NATIVE(compare_magnitude)(left, right) * (left->negative ? -1 : 1);
    *output = result;
    return ama_status_success();
}

/** Each column's sum is at most 511, so unsigned carry is exactly zero or one. */
static void AMA_NATIVE(sum_magnitudes)(AMA_NATIVE(State) *result,
    const AMA_NATIVE(State) *left, const AMA_NATIVE(State) *right) {
    unsigned carry = 0;
    for (size_t index = 0; index < result->count; index++) {
        unsigned sum = carry + (index < left->count ? left->limbs[index] : 0)
            + (index < right->count ? right->limbs[index] : 0);
        result->limbs[index] = (unsigned char)(sum % 256);
        carry = sum / 256;
    }
}

/** Subtract the smaller magnitude. Borrow is zero/one; adding 256 keeps every
 * intermediate nonnegative and within 511 without signed arithmetic.
 */
static void AMA_NATIVE(subtract_magnitudes)(AMA_NATIVE(State) *result,
    const AMA_NATIVE(State) *larger, const AMA_NATIVE(State) *smaller) {
    unsigned borrow = 0;
    for (size_t index = 0; index < result->count; index++) {
        unsigned limb = larger->limbs[index];
        unsigned subtrahend = borrow + (index < smaller->count ? smaller->limbs[index] : 0);
        result->limbs[index] = (unsigned char)((limb + 256 - subtrahend) % 256);
        borrow = limb < subtrahend;
    }
}

/** Prepare a disjoint native result before finalizing any aliased source. */
AmaStatus AMA_NATIVE(add)(AMA_NATIVE(State) *destination,
    const AMA_NATIVE(State) *left, const AMA_NATIVE(State) *right) {
    if (!destination->live || !left->live || !right->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    bool same_sign = left->negative == right->negative;
    const AMA_NATIVE(State) *larger = left, *smaller = right;
    if (!same_sign && AMA_NATIVE(compare_magnitude)(left, right) < 0) { larger = right; smaller = left; }
    size_t count = same_sign ? (left->count > right->count ? left->count : right->count) : larger->count;
    if (same_sign) {
        if (count >= PTRDIFF_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        count++;
    }
    AMA_NATIVE(State) prepared;
    AmaStatus status = AMA_NATIVE(prepare)(&prepared, count);
    if (status.code != AMA_OK) { return status; }
    prepared.negative = larger->negative;
    if (same_sign) { AMA_NATIVE(sum_magnitudes)(&prepared, left, right); }
    else { AMA_NATIVE(subtract_magnitudes)(&prepared, larger, smaller); }
    AMA_NATIVE(normalize)(&prepared);
    prepared.live = true;
    status = AMA_NATIVE(destroy)(destination);
    if (status.code != AMA_OK) { (void)AMA_NATIVE(destroy)(&prepared); return status; }
    *destination = prepared;
    return ama_status_success();
}
