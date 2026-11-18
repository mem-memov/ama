/** @file
 * @brief Exhaust finite occurrence orders against an independent stable insertion oracle.
 */
/** An independent lexicographic permutation enumerator, unrelated to merges. */
static bool next_order(size_t *order, size_t count) {
    size_t pivot = count;
    while (pivot > 1 && order[pivot - 2] >= order[pivot - 1]) { pivot--; }
    if (pivot <= 1) { return false; }
    pivot -= 2;
    size_t successor = count - 1;
    while (order[successor] <= order[pivot]) { successor--; }
    size_t temporary = order[pivot]; order[pivot] = order[successor]; order[successor] = temporary;
    for (size_t left = pivot + 1, right = count - 1; left < right; left++, right--) {
        temporary = order[left]; order[left] = order[right]; order[right] = temporary;
    }
    return true;
}

/** Exhaust all 120 five-record permutations and their 0..5 prefixes against an
 * independent stable insertion oracle; prove identity/stability/no allocation.
 */
static void it_orders_every_five_record_permutation_and_prefix_stably_without_allocation(void) {
    setup_static(static_scope_text, static_scope_text);
    size_t order[] = {0, 1, 2, 3, 4}, keys[] = {0, 1, 1, 2, 2}, checked = 0;
    do {
        for (size_t count = 0; count <= 5; count++) {
            StaticOccurrence records[5] = {{0}};
            StaticOccurrence *expected[5] = {0};
            StaticModule module = {.occurrence_count = count};
            for (size_t index = 0; index < count; index++) {
                records[index].site = (SourceSpan){project.entry->source->text, keys[order[index]], keys[order[index]] + 1};
                records[index].next = index + 1 < count ? &records[index + 1] : NULL;
                expected[index] = &records[index];
                for (size_t position = index; position > 0
                    && expected[position - 1]->site.start > expected[position]->site.start; position--) {
                    StaticOccurrence *temporary = expected[position];
                    expected[position] = expected[position - 1]; expected[position - 1] = temporary;
                }
            }
            module.first_occurrence = count == 0 ? NULL : &records[0];
            size_t before = allocation_count;
            static_sort_occurrences(&module);
            StaticOccurrence *record = module.first_occurrence;
            for (size_t index = 0; index < count; index++) { assert(record == expected[index]); record = record->next; }
            assert(record == NULL && module.last_occurrence == (count == 0 ? NULL : expected[count - 1]));
            assert(allocation_count == before && module.occurrence_count == count);
            checked++;
        }
    } while (next_order(order, 5));
    assert(checked == 720);
    destroy_static();
}
