/** @file
 * @brief Compile fixed swap/length methods and their full selected result lifetimes.
 */
#include "../execution/common.c"
#include "../list-insertion/support/integer.c"
static bool fail_size_bridge;
static bool fail_selected_integer;

/** Inspect an owned integer result through a declared native parameter. */
static bool ordering_native_entry(FILE *output, const ScaffoldType *type) {
    const char *logical = type->plan->type->module->source->logical_path;
    int written;
    if (strcmp(logical, "//integer") == 0) {
        written = fputs(fail_selected_integer
            ? "puts(\"selected-failed\"); *status=ama_status_make(AMA_NATIVE_FAILURE,NULL,0); return;\n"
            : "printf(\"length=%zu\\n\",state->number);\n", output);
    } else if (strcmp(logical, "/test/fixture/v1.0.0/src/inspect") == 0 || strcmp(logical, "/test/fixture/v1.0.0/src/read") == 0) {
        written = fprintf(output, "printf(\"%s=%%zu\\n\",parameters->AMA_INPUT(0)->payload->number);\n",
            strcmp(logical, "/test/fixture/v1.0.0/src/read") == 0 ? "item" : "length");
    } else { return false; }
    assert(written != EOF);
    return true;
}
/** Make the single selected bridge observable; no default or literal may run
 * for the fresh result. All writes remain confined to this test's own fixture.
 */
static void ordering_standard(void) {
    insertion_standard();
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("inspect", "@<\ninput\n//integer\n@C\n");
    native_fixture("read", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@C\n");
    file("generated/standard/integer.h", fail_size_bridge
        ? "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; bool negative; };\n"
          "static inline AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t number) {"
          "(void)destination;(void)number;puts(\"size-failed\");return ama_status_make(AMA_BUSY,NULL,0); }\n"
        : "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; bool negative; };\n"
          "static inline AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t number) {"
          "printf(\"size=%zu\\n\",number);*destination=(AMA_NATIVE(State)){.number=number,.address=destination};return ama_status_success(); }\n");
    native_entry_override = ordering_native_entry;
}
/** Sibling methods see the current shape and direct result input copies survive
 * ephemeral result destruction; later length calls observe the changed shape.
 */
static void it_executes_swap_and_length_siblings_with_owned_result_cleanup(void) {
    ordering_standard();
    execute_project("@!\nzero\n//integer\n |0\none\n//integer\n |1\nitems[]\n/test/fixture/v1.0.0/src/value\n |10\n |20\ninspect\n/test/fixture/v1.0.0/src/inspect\nread\n/test/fixture/v1.0.0/src/read\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "items[]\n swap !\n  <\n  < one\n length !\n  > result\nread !\n < items[zero]\nread !\n < items[one]\n"
        "items[]\n length !\n  value !\n clear !\n length !\n  value !\ninspect !\n < result\n",
        "size=2\nitem=20\nitem=10\nsize=2\nlength=2\nsize=0\nlength=0\nlength=2\n", true);
}
/** Either invalid position refuses before mutation and a local handler releases
 * the entire service, so a following fresh length service still succeeds.
 */
static void it_rejects_both_swap_positions_without_changing_length(void) {
    ordering_standard();
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\n |10\ninspect\n/test/fixture/v1.0.0/src/inspect\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "items[]\n swap !\n  < |1\n :\n  probe !\n   < |12\nitems[]\n swap !\n  <\n  < |-1\n :\n  probe !\n   < |1\nitems[]\n length !\n  value !\n",
        "12:1:12\n1:2:1\nsize=1\nlength=1\n", true);
}
/** A native constructor's Busy remains technical; no selected output or colon
 * handler may execute, and its unsuccessful payload has no destroy credit.
 */
static void it_keeps_length_bridge_failure_technical(void) {
    fail_size_bridge = true;
    ordering_standard();
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\ninspect\n/test/fixture/v1.0.0/src/inspect\nprobe\n/test/fixture/v1.0.0/src/worker\nitems[]\n length !\n  value !\n :\n  probe !\n   < |999\n",
        "size-failed\n", false);
    expect_diagnostic("Ama runtime failure: 9");
    fail_size_bridge = false;
}
/** Selected result body failure remains technical and skips pending siblings. */
static void it_cleans_length_result_after_selected_body_failure(void) {
    fail_selected_integer = true;
    ordering_standard();
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nprobe\n/test/fixture/v1.0.0/src/worker\nitems[]\n length !\n  value !\n  :\n   probe !\n    < |999\n clear !\n",
        "size=0\nselected-failed\n", false);
    expect_diagnostic("Ama runtime failure: 4");
    fail_selected_integer = false;
}
/** Run actual source-to-C execution and clear callbacks before fixture teardown. */
void run(void) {
    it_executes_swap_and_length_siblings_with_owned_result_cleanup();
    it_rejects_both_swap_positions_without_changing_length();
    it_keeps_length_bridge_failure_technical();
    it_cleans_length_result_after_selected_body_failure();
    native_hook_override = NULL;
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
/** Return only after every compiled project and diagnostic has matched. */
int main(void) { run(); return EXIT_SUCCESS; }
