/** @file
 * @brief Distinguish omitted and explicit owning sources and verify the exact standard integer capability profile.
 */
#include "../../diagnostic-evidence.c"

/** Runtime emptiness never excuses an explicit owning copy, while omissions and reference-list handle copies require none. */
static void it_requires_element_copy_only_for_explicit_owning_append_insert_sources(void) {
    prepare_lists();
    const char *valid[] = {
        "@!\nitems[]\n.resource\nitems[]\n append !\n insert !\n  <\n  <\n remove !\n clear !\n swap !\n length !\n  > count\n",
        "@!\nitems&[]\n.resource\nreference&\n.resource\nitems&[]\n append !\n  < reference&\n  > result&\n insert !\n  source& < reference&\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) { expect_list(valid[index], true); }
    const char *invalid[] = {
        "@!\nitems[]\n.resource\nreference&\n.resource\nitems[]\n append !\n  < reference&\n",
        "@!\nitems[]\n.resource\nreference&\n.resource\nitems[]\n insert !\n  source& < reference&\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        SemanticStatus status = resolve_list(invalid[index]);
        assert(status == SEMANTIC_INVALID && strstr(semantic.error.message, "independently copyable elements") != NULL);
        assert(strcmp(semantic.error.subject, ".resource") == 0);
        destroy_semantic();
    }
}
/** No project impostor, Ama body or incomplete native profile can satisfy the fixed standard integer contract. */
static void it_requires_the_exact_standard_native_integer_profile_and_bridge_obligation(void) {
    prepare_lists();
    for (unsigned bits = 0; bits < 8; bits++) {
        char abi[160] = "+ initialize\n- destroy\n";
        if (bits & 1u) { strcat(abi, "= clone\n"); }
        if (bits & 2u) { strcat(abi, "| literal\n"); }
        if (bits & 4u) { strcat(abi, "[] index\n"); }
        integer_fixture("@C\n", abi);
        expect_list("@!\nitems[]\n.value\nitems[]\n clear !\n", bits == 7);
    }
    SemanticStatus status = resolve_list("@!\nitems[]\n.value\nposition\n.integer\nitems[]\n insert !\n  < position\n");
    assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
    assert(semantic_nominal_note(&semantic.error,".integer") && semantic_nominal_note(&semantic.error,"..integer"));
    destroy_semantic();
    integer_fixture("@!\n", "+ initialize\n- destroy\n");
    expect_list("@!\nitems[]\n.value\nitems[]\n clear !\n", false);
    integer_fixture("@<\nresource\n.resource\n@C\n", "+ initialize\n- destroy\n= clone\n| literal\n[] index\n");
    expect_list("@!\nitems[]\n.value\nitems[]\n clear !\n", false);
    file("generated/program/main", "@!\nitems[]\n.value\nitems[]\n clear !\n");
    setup_project();
    ProjectStatus loaded = load_root("generated/program");
    assert(loaded == PROJECT_MISSING && !project.valid && diagnostic_valid(&project.error));
    destroy_project();
}
