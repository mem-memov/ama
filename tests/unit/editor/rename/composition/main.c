/** @file
 * @brief Verify freshly revalidated all-file document transactions and every measured rename allocation failure.
 */
#include "../common.c"
/** An exclusively owned editor document store borrows versioned snapshot bytes for this fixture. */
typedef struct {
    const ModuleSource *sources[16];
    const SourceText *current[16];
    size_t count;
    size_t calls;
    bool reject;
} Documents;
/** Initialize every document version before source publication; fixed fixture capacity is checked explicitly. */
static Documents documents(void) {
    Documents store = {0};
    for (const ModuleSource *source = analysis.frontend.project.catalog.first; source != NULL; source = source->next) {
        assert(store.count < sizeof(store.sources)/sizeof(*store.sources));
        store.sources[store.count] = source;
        store.current[store.count++] = source->text;
    }
    return store;
}
/** Exact document identity is retained independently of its current version pointer. */
static size_t document(const Documents *store, const ModuleSource *source) {
    for (size_t index = 0; index < store->count; index++) { if (store->sources[index] == source) { return index; } }
    return store->count;
}
/** Validate all expected versions before any infallible pointer publication under exclusive ownership.
 * Rejection changes no document; no allocation/failure point exists after the preflight loop.
 */
static bool publish(void *context, const EditorRenameFile *files) {
    Documents *store = context;
    store->calls++;
    if (store->reject) { return false; }
    for (const EditorRenameFile *file = files; file != NULL; file = file->next) {
        size_t index = document(store,file->original);
        if (index == store->count || store->current[index] != file->original->text) { return false; }
    }
    for (const EditorRenameFile *file = files; file != NULL; file = file->next) {
        store->current[document(store,file->original)] = file->replacement;
    }
    return true;
}
/** Multi-file validation and one atomic callback change only the selected callee field's declaration/references. */
static void it_publishes_all_versioned_documents_together_and_preserves_rejected_transactions(void) {
    project_fixture("@!\nworker\n/test/fixture/v1.0.0/src/worker\nargument\n/test/fixture/v1.0.0/src/cell\nworker !\n argument < argument\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/external","@!\nworker\n/test/fixture/v1.0.0/src/worker\nargument\n/test/fixture/v1.0.0/src/cell\nworker !\n argument < argument\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/worker","argument","new argument"));
    Documents store = documents();
    store.reject = true;
    assert(!editor_rename_publish(&rename_plan,publish,&store) && store.calls == 1);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_IO_FAILURE);
    for (size_t index = 0; index < store.count; index++) { assert(store.current[index] == store.sources[index]->text); }
    store.reject = false;
    assert(editor_rename_publish(&rename_plan,publish,&store) && store.calls == 2 && rename_plan.validated);
    for (const EditorRenameFile *file = rename_plan.first; file != NULL; file = file->next) {
        assert(store.current[document(&store,file->original)] == file->replacement);
    }
    assert(!editor_rename_publish(&rename_plan,publish,&store) && store.calls == 3);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_IO_FAILURE);
    assert(changed_file("/test/fixture/v1.0.0/src/external") != NULL && text_is(rename_plan.target.definition,"argument"));
    store = (Documents){0};
    destroy_rename();
}
/** Prior validation is not a lease: publication must recheck versions and must never call the writer after a changed input. */
static void it_revalidates_immediately_before_publication_and_never_calls_a_writer_on_failure(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed") && validate());
    Documents store = documents();
    file("generated/project/packages/test/fixture/v1.0.0/src/main","@!\nchanged\n/test/fixture/v1.0.0/src/cell\n");
    assert(!editor_rename_publish(&rename_plan,publish,&store) && store.calls == 0);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_CHANGED_INPUT && !rename_plan.validated);
    store = (Documents){0};
    destroy_rename();
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed"));
    store = documents();
    failure_at = allocation_count+1;
    assert(!editor_rename_publish(&rename_plan,publish,&store) && store.calls == 0);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
    failure_at = 0;
    assert(editor_rename_publish(&rename_plan,publish,&store) && store.calls == 1);
    store = (Documents){0};
    destroy_rename();
}
/** A same-name request changes no source and therefore publishes no document transaction. */
static void it_skips_document_publication_for_same_name_requests(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","value"));
    Documents store = documents();
    assert(editor_rename_publish(&rename_plan,publish,&store) && store.calls == 0);
    assert(point(analysis.selected->text,"value") != 0);
    store = (Documents){0};
    destroy_rename();
}
/** Every allocation in full workspace preparation, name/file construction and candidate identity validation is an explicit failure point. */
static void it_unwinds_every_workspace_plan_and_candidate_allocation_prefix(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\ncopy\n value\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed value") && validate());
    size_t total = allocation_count;
    destroy_rename();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_editor();
        editor_rename_init(&rename_plan,sources.allocator);
        failure_at = failure;
        ModuleRoot root = {"generated/project",NULL,0,MODULE_ROOT_PROJECT};
        bool valid = editor_workspace_build(&analysis,&root,1,"/test/fixture/v1.0.0/src/main");
        Diagnostic *diagnostic = &analysis.diagnostic;
        if (valid) {
            valid = prepare("/test/fixture/v1.0.0/src/main","value","renamed value");
            diagnostic = &rename_plan.diagnostic;
            if (valid) { valid = editor_rename_validate(&rename_plan); }
        }
        if (valid || diagnostic->code != DIAGNOSTIC_OUT_OF_MEMORY) {
            fprintf(stderr,"Rename allocation %zu/%zu: valid=%u code=%u message=%s\n",failure,total,
                (unsigned)valid,(unsigned)diagnostic->code,diagnostic->message == NULL ? "none" : diagnostic->message);
        }
        assert(!valid && !rename_plan.validated && diagnostic->code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_rename();
    }
}
/** Copy an explicit maintained native text implementation into a separate test-owned standard root. */
static void copy_native_text(void) {
    directory("generated/standard");
    directory("generated/standard/text+");
    directory("generated/standard/text+/native");
    const char *files[] = {"text","text.h","text.c","text.abi","text+/native/internal.h", "text+/native/unicode.c","text+/native/storage.c","text+/native/values.c"};
    for (size_t index = 0; index < sizeof(files)/sizeof(*files); index++) {
        char source[4096], target[4096], bytes[32768];
        int length = snprintf(source,sizeof(source),"%s/%s",TEST_STANDARD_ROOT,files[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        length = snprintf(target,sizeof(target),"generated/standard/%s",files[index]);
        assert(length > 0 && (size_t)length < sizeof(target));
        FILE *input = fopen(source,"rb");
        assert(input != NULL);
        size_t count = fread(bytes,1,sizeof(bytes)-1,input);
        assert(!ferror(input) && count < sizeof(bytes)-1);
        int closed = fclose(input);
        assert(closed == 0);
        bytes[count] = 0;
        file(target,bytes);
    }
}
/** Even a harmless native comment edit changes a captured dependency and prevents publication of a stale plan. */
static void it_rejects_changed_native_companion_inputs_before_source_publication(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\nmessage\n//text\n |text\n");
    copy_native_text();
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},{"generated/standard",NULL,0,MODULE_ROOT_STANDARD}};
    setup_rename_roots(roots,2);
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed") && validate());
    FILE *output = fopen("generated/standard/text.c","ab");
    assert(output != NULL);
    int written = fputs("\n/* Changed native dependency. */\n",output);
    int closed = fclose(output);
    assert(written != EOF && closed == 0);
    Documents store = documents();
    assert(!editor_rename_publish(&rename_plan,publish,&store) && store.calls == 0);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_CHANGED_INPUT && !rename_plan.validated);
    assert(strcmp(diagnostic_code_name(rename_plan.diagnostic.code),"changed-input") == 0);
    assert(rename_plan.diagnostic.subject != NULL && strstr(rename_plan.diagnostic.subject,"text.c") != NULL);
    store = (Documents){0};
    destroy_rename();
}
/** Exercise versioned source application independently from raw name editing and semantic-capture checks. */
void run(void) {
    it_publishes_all_versioned_documents_together_and_preserves_rejected_transactions();
    it_revalidates_immediately_before_publication_and_never_calls_a_writer_on_failure();
    it_skips_document_publication_for_same_name_requests();
    it_unwinds_every_workspace_plan_and_candidate_allocation_prefix();
    it_rejects_changed_native_companion_inputs_before_source_publication();
}
/** Candidate snapshots and every unsuccessful replacement prefix retire before a successful exit. */
int main(void) { run(); return EXIT_SUCCESS; }
