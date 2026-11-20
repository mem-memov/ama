/** @file
 * @brief Check deep real expression traversal without recursive C activation.
 */
/** Source nesting is represented by explicit AST/event work, not C activation
 * depth. The fixed fixture size is not a parser or semantic nesting limit.
 */
static void it_collects_deep_expression_owners_without_recursive_c_activation(void) {
    size_t depth = 600;
    const char *head = "@...\n.value\n.type\n", *binding = ".slot < .type\n";
    size_t size = strlen(head) + depth * strlen(binding) + depth * (depth + 1) / 2 + 1;
    char *text = malloc(size);
    assert(text != NULL);
    size_t used = strlen(head);
    memcpy(text, head, used);
    for (size_t level = 1; level <= depth; level++) {
        memset(text + used, ' ', level); used += level;
        memcpy(text + used, binding, strlen(binding)); used += strlen(binding);
    }
    assert(used + 1 == size); text[used] = 0;
    setup_static(text, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    assert(module->occurrence_count == depth + 1);
    const StaticOccurrence *record = module->first_occurrence;
    for (size_t index = 0; index <= depth; index++) {
        assert(record != NULL && layout_text_is(record->site, ".type"));
        assert(record->root == (index == 0) && record->receiver == NULL);
        record = record->next;
    }
    assert(record == NULL);
    destroy_static();
    free(text);
}
