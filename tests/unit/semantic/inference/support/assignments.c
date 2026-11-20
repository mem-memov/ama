/** @file
 * @brief Check fresh assignment classification, all transfer shapes and exact rejection sites.
 */
/** The executable source determines type but never becomes the inferred field's creation-time initializer. */
static void it_types_a_first_assignment_receiver_without_synthetic_declaration_or_initializer(void) {
    prepare_inference();
    valid_inference("@!\nis_ok\n.value\n |0\nnew_var\n is_ok\n");
    SemanticSymbol *created = field("new_var");
    assert(created != NULL && created->inferred && created->declaration == NULL);
    assert(created->initializer_count == 0 && created->initializer_syntax == NULL);
    assert(created->type == field("is_ok")->type && !created->reference && !created->list);
    assert(created->ordinal == 1 && created->role_ordinal == 1 && layout_text_is(created->site, "new_var"));
    SemanticStatus resolved = semantic_resolve(&semantic);
    assert(resolved == SEMANTIC_OK && semantic.resolved && created->initializers == NULL);
    destroy_semantic();
}
/** The source table distinguishes references, whole/indexed lists, continuation copies and lexical self. */
static void it_derives_the_complete_assignment_source_storage_table_immediately(void) {
    prepare_inference();
    valid_inference("@!\nowned\n.value\n |1\nreference&\n.value\n &owned\nitems[]\n.value\n |1\nreferences&[]\n.value\n &owned\nindex\n.value\n |0\n"
        "copy\n owned\nreference copy&\n reference&\nacquired&\n &owned\nlist copy[]\n items[]\nreference list copy&[]\n references&[]\n"
        "element\n items[index]\nreference element&\n references&[index]\ncontinuation copy\n % reference&\nself&\n &\n");
    const char *names[] = {"copy", "reference copy", "acquired", "list copy", "reference list copy",
        "element", "reference element", "continuation copy", "self"};
    const bool references[] = {false, true, true, false, true, false, true, false, true};
    const bool lists[] = {false, false, false, true, true, false, false, false, false};
    for (size_t index = 0; index < 9; index++) {
        SemanticSymbol *symbol = field(names[index]);
        assert(symbol != NULL && symbol->inferred && symbol->reference == references[index] && symbol->list == lists[index]);
        assert(symbol->type == (index == 8 ? semantic_find_type(&semantic,".main") : field("owned")->type));
        assert(symbol->initializer_count == 0 && symbol->initializers == NULL);
    }
    destroy_semantic();
    prepare_inference();
    valid_inference("@!\nproject value\n.value\nstandard value\n..value\nfirst\n project value\nsecond\n standard value\n");
    assert(field("first")->type != field("second")->type);
    destroy_semantic();
}
/** First unresolved uses are never repaired by later receivers; later explicit declarations do not suppress a first receiver declaration. */
static void it_rejects_earlier_unresolved_uses_self_dependencies_and_later_explicit_collisions(void) {
    prepare_inference();
    const char *invalid[] = {
        "@!\nworker\n.worker\nworker !\n < unknown\n > unknown\n",
        "@!\nfirst\n first\n", "@!\nfirst\n second\nsecond\n first\n",
        "@!\nsource\n.value\nnew\n source\nnew\n.value\n",
        "@!\nsource\n.value\nnew&\n source\n", "@!\nsource[]\n.value\nnew\n source[]\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        SemanticStatus status = infer_text(invalid[index]);
        assert(status == SEMANTIC_INVALID && !semantic.inference_valid && diagnostic_valid(&semantic.error));
        if (index == 0) { assert(layout_text_is(semantic.error.primary.span, "unknown") && field("unknown") == NULL); }
        if (index == 3) {
            assert(semantic.error.related_count == 1);
            assert(semantic.error.primary.span.start > semantic.error.related[0].location.span.start);
        }
        destroy_semantic();
    }
    valid_inference("@!\ndestination\n.value\nother\n.value\ndestination\n later\nlater\n other\nlater\n.value\n");
    assert(field("later") != NULL && !field("later")->inferred);
    destroy_semantic();
}
