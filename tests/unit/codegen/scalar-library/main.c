/** @file
 * @brief Compile the maintained standard integer, its constants and typed list bridge.
 */
#include "../execution/common.c"

/** Observe the library's explicit checked conversion, without compiler payload casts. */
static bool scalar_entry(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".inspect") != 0) { return false; }
    int written = fputs("size_t number=0; AmaStatus converted="
        "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index"
        "(parameters->AMA_INPUT(0)->payload,&number);"
        "if(converted.code!=AMA_OK) { *status=converted;return; }"
        "printf(\"integer=%zu\\n\",number);\n", output);
    assert(written != EOF);
    return true;
}

/** Only project fixtures are authored; the real standard companions remain untouched. */
static void scalar_standard(void) {
    directory("generated/project");
    native_fixture("inspect", "@<\ninput\n..integer\n@C\n");
    native_entry_override = scalar_entry;
}

/** Length owns the real arbitrary-precision payload built by the exact size bridge. */
static void it_executes_real_integer_positions_and_length_results(void) {
    scalar_standard();
    execute_project("@!\nitems[]\n.value\n |5\nshow\n.inspect\nitems[]\n insert !\n  < |0\n append !\n length !\n  > count\nshow !\n < count\nitems[]\n remove !\n  < |1\n length !\n  > count\nshow !\n < count\n",
        "integer=3\ninteger=2\n", true);
}

/** Standard shared constants and ordinary copies remain the exact nominal integer. */
static void it_copies_shared_integer_constants_and_preserves_scalar_entry_state(void) {
    scalar_standard();
    execute_project("@!\nshow\n.inspect\n..integer\n one >> first\nsecond\n first\nsecond !\nshow !\n < second\n..integer\n zero >> first\nshow !\n < first\nshow !\n < second\n",
        "integer=1\ninteger=0\ninteger=1\n", true);
}

/** Arbitrarily large signed literals are valid integers but unavailable as indices. */
static void it_refuses_negative_and_above_host_indices_without_technical_failure(void) {
    scalar_standard();
    execute_project("@!\nitems[]\n.value\n |7\nnegative\n..integer\n |-1\nhuge\n..integer\n |99999999999999999999999999999999999999999999999999999999999999999999\nprobe\n.worker\n"
        "items[negative] !\n:\n probe !\n  < |1\nitems[huge] !\n:\n probe !\n  < |2\n",
        "1:1:1\n2:2:1\n", true);
}

/** Nonzero truth is independent of index representability; minus zero normalizes. */
static void it_checks_mathematical_truth_without_machine_integer_assumptions(void) {
    scalar_standard();
    execute_project("@!\nnegative\n..integer\n |-256\nzero\n..integer\n |-000\nprobe\n.worker\nnegative\n?\n probe !\n  < |1\nzero\n:\n probe !\n  < |2\n",
        "1:1:0\n2:2:0\n", true);
}

/** Invalid decimal bytes fail selected construction and never enter the root body. */
static void it_reports_malformed_integer_literals_with_the_standard_identity(void) {
    scalar_standard();
    execute_project("@!\nnumber\n..integer\n |12x\nprobe\n.worker\nprobe !\n < |999\n", "", false);
    expect_diagnostic("field construction, type ..integer");
}

/** A same-name project type needs no size bridge and never becomes ..integer. */
static void it_keeps_project_integer_names_and_standard_integer_types_distinct(void) {
    scalar_standard();
    native_fixture("integer", "@C\n");
    execute_project("@!\nitems[]\n.integer\nshow\n.inspect\nitems[]\n append !\n length !\n  > count\nshow !\n < count\n",
        "integer=1\n", true);
    file("generated/project/main", "@!\nsource\n..integer\n |1\ndestination\n.integer\ndestination\n source\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STDLIB, NULL, 0, MODULE_ROOT_STANDARD}};
    bool checked = compiler_frontend_check(&frontend, roots, 2, ".main");
    assert(!checked && frontend.specialization_initialized && !frontend.specializations.valid);
    assert(frontend.specializations.constraints.status == STATIC_CONSTRAINT_CONFLICT && !frontend.semantic_initialized);
    assert(diagnostic_valid(&frontend.error) && frontend.error.code == DIAGNOSTIC_TYPE_MISMATCH);
    destroy_frontend();
}

/** Equal mathematical data and shared protocols never imply nominal conversion. */
static void it_rejects_implicit_conversions_between_standard_scalar_types(void) {
    const char *names[] = {"..integer", "..std.index", "..std.condition"};
    for (size_t source = 0; source < 3; source++) {
        for (size_t destination = 0; destination < 3; destination++) {
            if (source == destination) { continue; }
            char text[256];
            int length = snprintf(text, sizeof(text), "@!\nsource\n%s\n |1\ndestination\n%s\ndestination\n source\n",
                names[source], names[destination]);
            assert(length > 0 && (size_t)length < sizeof(text));
            file("generated/project/main", text);
            setup_frontend();
            ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
                {TEST_STDLIB, NULL, 0, MODULE_ROOT_STANDARD}};
            bool checked = compiler_frontend_check(&frontend, roots, 2, ".main");
            assert(!checked && frontend.specialization_initialized && !frontend.specializations.valid);
            assert(frontend.specializations.constraints.status == STATIC_CONSTRAINT_CONFLICT && !frontend.semantic_initialized);
            assert(diagnostic_valid(&frontend.error) && frontend.error.code == DIAGNOSTIC_TYPE_MISMATCH);
            destroy_frontend();
        }
    }
}

/** The real standard-root model and all generated native signatures are exercised. */
void run(void) {
    it_executes_real_integer_positions_and_length_results();
    it_copies_shared_integer_constants_and_preserves_scalar_entry_state();
    it_refuses_negative_and_above_host_indices_without_technical_failure();
    it_checks_mathematical_truth_without_machine_integer_assumptions();
    it_reports_malformed_integer_literals_with_the_standard_identity();
    it_keeps_project_integer_names_and_standard_integer_types_distinct();
    it_rejects_implicit_conversions_between_standard_scalar_types();
    native_entry_override = NULL;
}

/** Return only after exact runtime output and diagnostics match. */
int main(void) { run(); return EXIT_SUCCESS; }
