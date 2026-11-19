/** @file
 * @brief Exercise real deeply nested static source expressions through the closure loader.
 */

/** Every written head has a distinct original span but shares one source
 * definition; fixture depth bounds test size rather than imposing a language cap.
 */
static void it_loads_deep_static_applications_with_one_definition_and_distinct_requests(void) {
    directory("generated/static depth");
    file("generated/static depth/node", "@<\nvalue\n...element\n@!\n");
    size_t depth = 768;
    const char *head = "@...\n...value\n.node\n", *binding = "...element < .node\n";
    size_t size = strlen(head) + depth * strlen(binding) + depth * (depth + 1) / 2 + 1;
    char *text = malloc(size);
    assert(text != NULL);
    size_t used = strlen(head);
    memcpy(text, head, used);
    for (size_t level = 1; level <= depth; level++) {
        memset(text + used, ' ', level); used += level;
        memcpy(text + used, binding, strlen(binding)); used += strlen(binding);
    }
    assert(used + 1 == size);
    text[used] = 0;
    file("generated/static depth/main", text);
    setup_project();
    assert(load_root("generated/static depth") == PROJECT_OK && module_count() == 2);
    const ProjectModule *node = project_find(&project, ".node");
    const ProjectDependency *edge = project.entry->first;
    size_t previous = 0;
    for (size_t index = 0; index <= depth; index++) {
        assert(edge != NULL && edge->target == node && !edge->implicit);
        assert(layout_text_is(edge->requirement, ".node") && edge->requirement.start > previous);
        previous = edge->requirement.start;
        edge = edge->next;
    }
    assert(edge == NULL);
    destroy_project();
    free(text);
}
