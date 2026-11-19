/** @file
 * @brief Verify full-key ordinary types, fixed lexical cells, nominal cycles and projection teardown.
 */
#include "../common.c"

/** One origin instantiated twice must retain different concrete input/output Types. */
static void it_preserves_distinct_captures_and_equal_key_reuse(void) {
    setup("@!\nnumber\n..integer\nflag\n..text\nfirst\n.other\nsecond\n.other\nagain\n.other\nfirst !\n < number\nsecond !\n < flag\nagain !\n < number\n",
        "@<\ninput\n...slot\n@>\noutput\n...slot\n@!\noutput\n input\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    SemanticType *first = field(main_type, "first")->type;
    SemanticType *second = field(main_type, "second")->type;
    assert(first != second && first->module == second->module);
    assert(!semantic_find_type(&semantics, ".other"));
    assert(!semantic_type(&semantics, field(main_type, "first")->type_site));
    assert(semantic_find_type(&semantics, ".main") == main_type);
    assert(semantic_type(&semantics, field(main_type, "number")->type_site) == field(main_type, "number")->type);
    assert(first == field(main_type, "again")->type);
    assert(field(first, "input")->type == field(main_type, "number")->type);
    assert(field(second, "output")->type == field(main_type, "flag")->type);
    assert(field(first, "input")->role == SYMBOL_PARAMETER && field(first, "input")->role_ordinal == 0);
    assert(field(first, "output")->role == SYMBOL_RESULT && field(first, "output")->role_ordinal == 0);
    assert(!semantics.native_valid && !semantics.inference_valid && !semantics.resolved);
    destroy();
}

/** Complete key ordering is independent from which generic call supplied evidence first. */
static void it_orders_full_keys_independently_of_call_evidence_order(void) {
    unsigned char *saved[2] = {NULL, NULL};
    size_t lengths[2] = {0, 0};
    for (size_t reverse = 0; reverse < 2; reverse++) {
        setup(reverse ? "@!\nnumber\n..integer\ntext\n..text\nfirst\n.other\nsecond\n.other\nfirst !\n < text\nsecond !\n < number\n"
            : "@!\nnumber\n..integer\ntext\n..text\nfirst\n.other\nsecond\n.other\nfirst !\n < number\nsecond !\n < text\n",
            "@<\ninput\n...slot\n@!\n", true);
        assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
        const IdentityName *previous = NULL;
        size_t count = 0;
        const ProjectModule *other = project_find(&project, ".other");
        for (const SemanticType *type = semantics.first; type; type = type->next) {
            if (type->module != other) { continue; }
            assert(count < 2);
            const IdentityName *name = type->specialization->context->registry_use->candidate->name;
            if (previous) {
                size_t common = previous->encoding_length < name->encoding_length ? previous->encoding_length : name->encoding_length;
                int order = memcmp(previous->encoding, name->encoding, common);
                assert(order < 0 || (order == 0 && previous->encoding_length < name->encoding_length));
            }
            if (!reverse) {
                lengths[count] = name->encoding_length;
                saved[count] = malloc(lengths[count]);
                assert(saved[count]);
                memcpy(saved[count], name->encoding, lengths[count]);
            } else {
                assert(lengths[count] == name->encoding_length);
                assert(memcmp(saved[count], name->encoding, lengths[count]) == 0);
            }
            previous = name;
            count++;
        }
        assert(count == 2);
        destroy();
    }
    free(saved[1]);
    free(saved[0]);
}

/** Existing ordinary checkers consume concrete generic calls and inferred outputs. */
static void it_resolves_concrete_generic_bodies_without_reclassifying_fields(void) {
    setup("@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n > fresh\n",
        "@<\ninput\n...slot\n@>\noutput\n...slot\n@!\noutput\n input\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    assert(semantics.type_closure == &analysis.closure && semantic_native(&semantics) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    SemanticSymbol *fresh = field(main_type, "fresh");
    size_t count = main_type->symbol_count;
    assert(semantic_resolve(&semantics) == SEMANTIC_OK && semantics.resolved);
    assert(semantics.inference_valid && main_type->symbol_count == count);
    assert(field(main_type, "fresh") == fresh && fresh->type == field(main_type, "known")->type);
    assert(main_type->statements && field(main_type, "worker")->type->statements);
    assert(semantic_relations(&semantics) == SEMANTIC_OK);
    destroy();
}

/** Reference and empty-list cycles are finite graph edges, not recursive expansion. */
static void it_links_nominal_cycles_before_ordinary_ownership_checks(void) {
    setup("@!\nnext&\n.main\nchildren[]\n.main\n", "@!\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    assert(main_type->module == project.entry && !main_type->next);
    assert(field(main_type, "next")->type == main_type && field(main_type, "next")->reference);
    assert(field(main_type, "children")->type == main_type && field(main_type, "children")->list);
    semantic_clone_capabilities(&semantics);
    assert(main_type->cloneable);
    destroy();
}

/** Projection keeps defaults and foreach locals separate from persistent ordinals. */
static void it_preserves_defaults_inferred_receivers_and_scoped_foreach_cells(void) {
    setup("@!\nseed\n..integer\n |7\ninferred\n seed\nitems[]\n..integer\nitems[item&]\n{\n copy&\n  item&\n}\n", "@!\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    assert(main_type->symbol_count == 4 && main_type->counts[SYMBOL_STATE] == 4);
    SemanticSymbol *seed = field(main_type, "seed");
    assert(seed->declaration && seed->initializer_count == 1 && seed->initializer_syntax);
    SemanticSymbol *inferred = field(main_type, "inferred");
    assert(inferred->inferred && !inferred->declaration && !inferred->initializer_count);
    assert(inferred->type == seed->type && inferred->ordinal == 1);
    SemanticForeachBinding *header = main_type->foreach_bindings;
    assert(header && !header->next && header->introduced);
    assert(header->collection == field(main_type, "items") && header->binding == main_type->locals);
    assert(header->binding->local && header->binding->reference && !header->binding->list);
    assert(header->binding->scope == header->syntax && header->binding->type == seed->type);
    assert(!semantic_symbol(main_type, header->binding->name));
    destroy();
}

/** A static assembly selects a runtime Type without itself becoming runtime storage. */
static void it_projects_selected_static_exports_without_producer_instances(void) {
    setup("@!\nvalue\n.other\n ...selected\n", "@...\n...selected\n..integer\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    assert(strcmp(field(main_type, "value")->type->module->source->logical_path, "..integer") == 0);
    for (SemanticType *type = semantics.first; type; type = type->next) {
        assert(type->module->syntax.sections.kind != MODULE_STATIC);
    }
    destroy();
}

/** Source interfaces retain independent ordinals and all written defaults. */
static void it_preserves_all_section_roles_and_native_protocol_consumption(void) {
    setup("@=\nconstant\n..integer\n |3\n@<\ninput\n..integer\n |4\n@>\noutput\n..integer\n |5\n@!\nstate\n..integer\n |6\n", "@!\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    const char *names[] = {"constant", "input", "output", "state"};
    for (size_t index = 0; index < 4; index++) {
        SemanticSymbol *item = field(main_type, names[index]);
        assert(item->role == (SymbolRole)index && item->ordinal == index);
        assert(item->role_ordinal == 0 && main_type->counts[index] == 1);
        assert(item->initializer_count == 1 && item->initializer_syntax);
    }
    assert(semantic_native(&semantics) == SEMANTIC_OK && semantics.native_valid);
    SemanticType *integer = field(main_type, "state")->type;
    assert(integer->abi.bindings[ABI_CLONE].present);
    semantic_clone_capabilities(&semantics);
    assert(main_type->cloneable && integer->cloneable);
    for (SemanticType *type = semantics.first; type; type = type->next) {
        assert(semantic_initializers(&semantics, type));
    }
    destroy();
}

/** Real noncopyable native protocol affects owning lists but not borrowed edges. */
static void it_composes_distinct_eager_and_clone_relations_on_projected_types(void) {
    const char *programs[] = {"@!\nowned[]\n.other\n", "@!\nborrowed&\n.other\n"};
    for (size_t index = 0; index < 2; index++) {
        directory(STATIC_SCOPE_ROOT_DIRECTORY);
        file("generated/static scope/other.h", "/* Fixture opaque resource payload. */\n");
        file("generated/static scope/other.c", "/* Fixture metadata checks execute no native body. */\n");
        file("generated/static scope/other.abi", "+ initialize\n- destroy\n");
        setup(programs[index], "@C\n", true);
        assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
        assert(semantic_native(&semantics) == SEMANTIC_OK);
        SemanticType *main_type = semantics.first;
        while (main_type->module != project.entry) { main_type = main_type->next; }
        semantic_clone_capabilities(&semantics);
        assert(main_type->cloneable == (index == 1));
        assert(!main_type->first->type->cloneable);
        assert(semantic_eager_graph(&semantics));
        assert(dependency_order(&semantics.eager) == GRAPH_OK);
        destroy();
    }
}

/** A supplied concrete argument cannot hide an unsupported generic default. */
static void it_checks_substituted_defaults_even_when_call_inputs_are_supplied(void) {
    for (size_t resource = 0; resource < 2; resource++) {
        directory(STATIC_SCOPE_ROOT_DIRECTORY);
        file("generated/static scope/resource", "@C\n");
        file("generated/static scope/resource.h", "/* Fixture native payload. */\n");
        file("generated/static scope/resource.c", "/* Fixture checks parse protocol metadata only. */\n");
        file("generated/static scope/resource.abi", "+ initialize\n- destroy\n= clone\n");
        setup(resource ? "@!\nknown\n.resource\nworker\n.other\nworker !\n < known\n"
            : "@!\nknown\n..integer\nworker\n.other\nworker !\n < known\n",
            "@<\ninput\n...slot\n |0\n@!\n", true);
        assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
        assert(semantic_native(&semantics) == SEMANTIC_OK);
        semantic_clone_capabilities(&semantics);
        bool success = true;
        for (SemanticType *type = semantics.first; success && type; type = type->next) {
            success = semantic_initializers(&semantics, type);
        }
        assert(success == !resource);
        if (resource) {
            assert(semantics.status == SEMANTIC_INVALID && diagnostic_valid(&semantics.error));
            assert(semantics.error.primary.span.source == project_find(&project, ".other")->source->text);
        }
        assert(!semantics.resolved);
        destroy();
    }
}

/** Definition-only closure does not allocate ordinary Types; unclosed input refuses. */
static void it_requires_closed_inputs_without_inventing_default_instances(void) {
    setup("@!\n", "@<\ninput\n...slot\n@!\n", false);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    assert(semantics.names_valid && !semantics.first);
    semantic_program_destroy(&semantics);
    semantic_program_init(&semantics, &project, sources.allocator);
    analysis.closure.closed = false;
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_INVALID);
    assert(!semantics.names_valid && !semantics.first);
    destroy();
}

/** Reuse and foreign-project inputs refuse without publishing a second inventory. */
static void it_refuses_reuse_and_foreign_project_provenance(void) {
    setup("@!\nnext&\n.main\n", "@!\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    SemanticType *original = semantics.first;
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_INVALID);
    assert(semantics.first == original && !semantics.names_valid);
    semantic_program_destroy(&semantics);
    Project foreign = project;
    semantic_program_init(&semantics, &foreign, sources.allocator);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_INVALID);
    assert(!semantics.names_valid && !semantics.first);
    destroy();
}

/** Every projection allocation failure preserves a destroyable ordinary prefix. */
static void it_unwinds_every_projection_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup("@!\nitems[]\n..integer\nitems[item&]\n{\n copied&\n  item&\n}\n", "@!\n", true);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        SemanticStatus status = semantic_specialization_names(&semantics, &analysis.closure);
        if (!cut) { assert(status == SEMANTIC_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SEMANTIC_OUT_OF_MEMORY && !semantics.names_valid); }
        failure_at = 0;
        destroy();
    }
    printf("specialized ordinary inventory allocation refusal positions: %zu\n", total);
}

/** Explicit dispatch covers concrete identity, source projection and resource boundaries. */
static void run(void) {
    it_preserves_distinct_captures_and_equal_key_reuse();
    it_orders_full_keys_independently_of_call_evidence_order();
    it_resolves_concrete_generic_bodies_without_reclassifying_fields();
    it_links_nominal_cycles_before_ordinary_ownership_checks();
    it_preserves_defaults_inferred_receivers_and_scoped_foreach_cells();
    it_projects_selected_static_exports_without_producer_instances();
    it_preserves_all_section_roles_and_native_protocol_consumption();
    it_composes_distinct_eager_and_clone_relations_on_projected_types();
    it_checks_substituted_defaults_even_when_call_inputs_are_supplied();
    it_requires_closed_inputs_without_inventing_default_instances();
    it_refuses_reuse_and_foreign_project_provenance();
    it_unwinds_every_projection_allocation_refusal();
}
/** Run every explicit assertion case and report success after counted teardown. */
int main(void) { run(); return 0; }
