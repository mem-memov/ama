/** @file
 * @brief Refuse every real allocation across whole static-inventory construction and validation.
 */

/** Failure injection begins after valid source/project preparation, covering
 * each static allocation and preserving source ownership and first diagnostics.
 */
static void it_unwinds_every_whole_static_collection_allocation_failure(void) {
    const char *source = "@<\n...leaf\n ...type\nfirst\n...family\n ...slot < ...leaf\n"
        "second\n...family\n ...slot < ...leaf\n@>\n...result\n...family\n@!\n.other\n ...out > ...alias\n";
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(source, STATIC_SCOPE_SOURCE_TEXT);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        StaticStatus status = static_inventory_collect(&inventory);
        if (failure == 0) { assert(status == STATIC_OK && inventory.valid); total = allocation_count - before; assert(total != 0); }
        else {
            assert(status == STATIC_OUT_OF_MEMORY && !inventory.valid && inventory.attempted);
            assert(diagnostic_valid(&inventory.error));
            assert(inventory.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        }
        destroy_static();
    }
}
