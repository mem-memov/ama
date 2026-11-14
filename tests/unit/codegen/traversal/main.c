/** @file
 * @brief Execute mutable traversal and copied continuation control through real C projects.
 */
#include "../execution/common.c"
#include "../list-insertion/support/integer.c"

/** Observe each selected stable payload and make duplicate visits observable. */
static bool traversal_entry(FILE *output, const ScaffoldType *type) {
    const char *logical = type->plan->type->module->source->logical_path;
    const char *body = strcmp(logical, ".show") == 0
        ? "printf(\"read=%zu\\n\",parameters->AMA_INPUT(0)->payload->number);\n"
        : strcmp(logical, ".value") == 0
        ? "printf(\"value=%zu\\n\",state->number);state->number++;\n" : NULL;
    if (body == NULL) { return false; }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}

/** Use a test-owned exact integer protocol and address-sensitive native values. */
static void traversal_standard(void) {
    insertion_standard();
    native_entry_override = traversal_entry;
}

/** Local bindings and reused reference fields select the original instances. */
static void it_executes_local_and_persistent_bindings(void) {
    traversal_standard();
    execute_project("@!\nzero\n..integer\n |0\nitems[]\n.value\n |4\n |5\ncurrent&\n.value\n"
        "items[item&]\n{\n item& !\n kept&\n  item&\n}\n"
        "items[current&]\n{\n current& !\n}\ncurrent& !\nkept& !\nitems[zero] !\n",
        "value=4\nvalue=5\nvalue=5\nvalue=6\nvalue=7\nvalue=8\nvalue=6\n", true);
}

/** Insertions at the frontier and at the end remain candidates in this pass. */
static void it_visits_new_future_memberships(void) {
    traversal_standard();
    execute_project("@!\nzero\n..integer\n |0\nitems[]\n.value\n |1\n |2\nextra\n.value\n |3\nlast\n.value\n |4\nonce\n.value\n |1\noff\n.value\n |0\n"
        "items[item&]\n{\n item& !\n once\n ?\n  items[]\n   insert !\n    < |1\n    < &extra\n   append !\n    < &last\n  once\n   off\n}\nextra !\nlast !\n",
        "value=1\nvalue=3\nvalue=2\nvalue=4\nvalue=3\nvalue=4\n", true);
}

/** Insertion behind the frontier is skipped, despite shifting later positions. */
static void it_skips_new_past_memberships(void) {
    traversal_standard();
    execute_project("@!\nzero\n..integer\n |0\nitems[]\n.value\n |1\n |2\nextra\n.value\n |3\nonce\n.value\n |1\noff\n.value\n |0\n"
        "items[item&]\n{\n item& !\n once\n ?\n  items[]\n   insert !\n    < zero\n    < &extra\n  once\n   off\n}\nitems[zero] !\n",
        "value=1\nvalue=2\nvalue=3\n", true);
}

/** Removing the selected membership refuses its body access before native code. */
static void it_guards_removed_origins_and_allows_explicit_detachment(void) {
    traversal_standard();
    execute_project("@!\nzero\n..integer\n |0\nfirst\n.value\n |1\nsecond\n.value\n |9\nrefs&[]\n.value\n &first\n"
        "refs&[item&]\n{\n item& !\n refs&[]\n  clear !\n item& !\n :\n  second !\n item&\n  &second\n item& !\n}\nfirst !\n",
        "value=1\nvalue=9\nvalue=10\nvalue=2\n", true);
}

/** A continuation clone retains its boundary but owns independent list elements. */
static void it_resumes_independent_copied_traversals(void) {
    traversal_standard();
    directory("generated/project");
    file("generated/project/task", "@!\nitems[]\n.value\n |1\n |2\nitems[item&]\n{\n %\n item& !\n}\n");
    execute_project("@!\nzero\n..integer\n |0\ntask\n.task\ntask !\nclone\n % task\ntask %\nclone %\ntask %\nclone %\n",
        "value=1\nvalue=1\nvalue=2\nvalue=2\n", true);
}

/** Whole-list copying maps active local references across sibling ownership roots. */
static void it_maps_active_bindings_across_a_copied_forest(void) {
    traversal_standard();
    directory("generated/project");
    native_fixture("show", "@<\ninput\n.value\n@C\n");
    file("generated/project/task", "@<\ntarget&\n.task\nid\n.value\n |0\n@>\ncode\n.value\n id\n@!\npeers&[]\n.task\nshow\n.show\n"
        "peers&[]\n clear !\n append !\n  < target&\npeers&[peer&]\n{\n %\n peer&\n  >> captured\n show !\n  < captured\n}\n");
    file("generated/project/forest", "@>\nsecond&\n.task\n@!\nzero\n..integer\n |0\none\n..integer\n |1\ntasks[]\n.task\ntasks[]\n append !\n append !\n"
        "tasks[zero] !\n < &tasks[one]\n < |1\ntasks[one] !\n < &tasks[zero]\n < |2\nsecond&\n &tasks[one]\n%\ntasks[zero] %\ntasks[one] %\n");
    execute_project("@!\nzero\n..integer\n |0\nforest\n.forest\nforest !\n > second&\ncopy\n % forest\nsecond& !\n <\n < |99\ncopy %\n",
        "read=2\nread=1\n", true);
}

/** Ordinary copy creates data without a resumable body or active traversal. */
static void it_rejects_resume_after_ordinary_copy(void) {
    traversal_standard();
    directory("generated/project");
    file("generated/project/task", "@!\nitems[]\n.value\n |7\nitems[item&]\n{\n %\n item& !\n}\n");
    execute_project("@!\nzero\n..integer\n |0\ntask\n.task\ntask !\ncopy\n task\ncopy %\n", "", false);
    expect_diagnostic("Ama runtime failure: 10");
}

/** Run real frontend, lowering, generated C, native lifecycle and runtime paths. */
void run(void) {
    it_executes_local_and_persistent_bindings();
    it_visits_new_future_memberships();
    it_skips_new_past_memberships();
    it_guards_removed_origins_and_allows_explicit_detachment();
    it_resumes_independent_copied_traversals();
    it_maps_active_bindings_across_a_copied_forest();
    it_rejects_resume_after_ordinary_copy();
    native_entry_override = NULL;
    native_hook_override = NULL;
    native_fixture_completion = NULL;
}

/** Return after exact observable output and resource cleanup have matched. */
int main(void) { run(); return EXIT_SUCCESS; }
