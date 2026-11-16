/** @file
 * @brief Exhaust the finite ABI capability/order domain independently of parser branch structure.
 */
/** Materialize one ordered subset and compare acceptance with the mandatory-slot predicate. */
static void check_order(const unsigned *order, size_t count) {
    static const char *const markers[] = {"+", "-", "|", "=", "?", "[]"};
    char text[160] = {0};
    size_t length = 0;
    unsigned mask = 0;
    for (size_t index = 0; index < count; index++) {
        int written = snprintf(text + length, sizeof(text) - length, "%s same_identifier\n", markers[order[index]]);
        assert(written > 0 && (size_t)written < sizeof(text) - length);
        length += (size_t)written;
        mask |= 1u << order[index];
    }
    setup_test();
    ParseStatus status = parse_abi(text);
    bool expected = (mask & 3u) == 3u;
    assert((status == PARSE_OK) == expected && table.valid == expected);
    if (!expected) {
        assert(diagnostic_valid(&table.error.diagnostic));
        assert(table.error.diagnostic.primary.span.start == length);
    } else {
        for (size_t index = 0; index < ABI_BINDING_COUNT; index++) {
            assert(table.bindings[index].present == ((mask & (1u << index)) != 0));
            if (table.bindings[index].present) { assert(layout_text_is(table.bindings[index].function, "same_identifier")); }
        }
    }
    destroy_abi();
}
/** Enumerate every permutation of every subset once; recursion has the fixed six-slot bound. */
static size_t enumerate_orders(unsigned *order, size_t count, unsigned mask) {
    check_order(order, count);
    size_t visited = 1;
    for (unsigned index = 0; index < ABI_BINDING_COUNT; index++) {
        if ((mask & (1u << index)) == 0) {
            order[count] = index;
            visited += enumerate_orders(order, count + 1, mask | (1u << index));
        }
    }
    return visited;
}
/** Acceptance depends on the slot set, never order or spelling-based capability discovery. */
static void it_exhausts_all_1957_ordered_capability_subsets(void) {
    unsigned order[ABI_BINDING_COUNT] = {0};
    size_t visited = enumerate_orders(order, 0, 0);
    assert(visited == 1957);
}
