/** @file
 * @brief Check complete static spellings without symbol lookup or allocation.
 */
/** Check complete static-name spellings independently of concrete module paths;
 * source storage/allocations are unchanged by recognition.
 */
static void it_recognizes_atomic_static_names_without_module_lookup(void) {
    const char *valid[] = {"...type", "...key type.element-2", "...ключ-2.值", "..._item-_value"};
    const char *invalid[] = {"...", "....type", "..type", ".type", "... type",
        "...type.", "...type..part", "...type .part", "...type. part", "...type--part"};
    for (size_t group = 0; group < 2; group++) {
        const char *const *names = group == 0 ? valid : invalid;
        size_t count = group == 0 ? sizeof(valid) / sizeof(*valid) : sizeof(invalid) / sizeof(*invalid);
        for (size_t index = 0; index < count; index++) {
            setup_tree();
            const SourceText *source;
            size_t length = strlen(names[index]);
            assert(source_manager_add(&sources, "static-name",
                (const unsigned char *)names[index], length, &source) == SOURCE_OK);
            SourceSpan text = {source, 0, length};
            size_t allocations = allocation_count;
            assert(parser_static_name(text) == (group == 0));
            assert(allocation_count == allocations);
            if (group == 0) { assert(!parser_path(text)); }
            destroy_tree();
        }
    }
    setup_tree();
    assert(parse_text("@!\nvalue\n.module\n") == PARSE_OK);
    destroy_tree();
}
