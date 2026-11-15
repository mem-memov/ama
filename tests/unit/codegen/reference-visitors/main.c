/** @file
 * @brief Execute checked native reference visits without exposing stable-pool addresses or extending view lifetimes.
 */
#include "../execution/common.c"

/** Observe callback admission separately from the value read; failed access must never enter user code. */
static bool reference_entry(FILE *body, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".reader") != 0) { return false; }
    int written = fputs("size_t value=0; AmaStatus observed=parameters->AMA_INPUT(0)->with_read("
        "parameters->AMA_INPUT(0)->storage,AMA_NATIVE(inspect),&value);"
        "printf(\"visit=%u:%zu\\n\",(unsigned)(observed.code==AMA_OK),value);\n", body);
    assert(written != EOF);
    return true;
}

/** Author a nominal callback from the checked scaffold model, then include the ordinary typed test implementation. */
static void reference_callback(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, ".reader") != 0) { continue; }
        const ScaffoldType *target = native_scaffold_type(scaffold, type->members->field->symbol->type);
        char saved[512];
        int length = snprintf(saved, sizeof(saved), "%s.saved", type->body_path);
        assert(length > 0 && (size_t)length < sizeof(saved));
        int moved = rename(type->body_path, saved);
        assert(moved == 0);
        FILE *body = fopen(type->body_path, "wb");
        assert(body != NULL);
        bool included = scaffold_include(body, type->interface_name) && scaffold_include(body, type->begin_name);
        assert(included);
        int written = fprintf(body, "static AmaStatus AMA_NATIVE(inspect)(const %s *view,void *resource) {"
            "*(size_t *)resource=view->payload->number; return ama_status_success(); }\n",
            target->names[SCAFFOLD_READ]->internal_name);
        assert(written > 0);
        included = scaffold_include(body, type->end_name);
        assert(included);
        const char *basename = strrchr(saved, '/');
        included = scaffold_include(body, basename == NULL ? saved : basename + 1);
        assert(included);
        int closed = fclose(body);
        assert(closed == 0);
    }
}

/** A checked reference permits a synchronous read and rejects an empty target without invoking its visitor. */
static void it_visits_live_references_and_refuses_empty_ones(void) {
    directory("generated/project");
    native_fixture("reader", "@<\ntarget&\n.value\n@C\n");
    native_entry_override = reference_entry;
    native_fixture_completion = reference_callback;
    fprintf(stderr, "visitor scenario: %s\n", __func__);
    execute_project("@!\nvalue\n.value\n |7\nreader\n.reader\nreader !\n < &value\nreader !\n", "visit=1:7\nvisit=0:0\n", true);
}

/** Parent replacement retires its owned child; the old checked reference must not resolve to replacement storage. */
static void it_refuses_retired_children_and_visits_the_fresh_child(void) {
    file("generated/project/box", "@>\nvalue&\n.value\n@!\nchild\n.value\n |99\nvalue&\n &child\n");
    fprintf(stderr, "visitor scenario: %s\n", __func__);
    execute_project("@!\nbox\n.box\nreplacement\n.box\nreader\n.reader\nbox !\n > old&\nbox\n replacement\nreader !\n < old&\nbox !\n > fresh&\nreader !\n < fresh&\n", "visit=0:0\nvisit=1:99\n", true);
}

/** Exercise both admission and invalidation through compiled generated C, with no generated text as an oracle. */
void run(void) {
    it_visits_live_references_and_refuses_empty_ones();
    it_refuses_retired_children_and_visits_the_fresh_child();
    expect_diagnostic("");
}

/** Report success only after all real native visitor compositions finish. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
