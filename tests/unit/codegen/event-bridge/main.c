/** @file
 * @brief Execute native completion import through actual selected Ama methods and typed results.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * requires normal generated access rather than callback mutation of Ama instances.
 */
#include "../execution/common.c"
/** Read maintained source bytes without inspecting generated C or SDK artifacts. */
static void fixture(const char *name, char source[16384]) {
    char path[256];
    int length = snprintf(path, sizeof(path), "project/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(source, 1, 16383, input);
    assert(count < 16383 && !ferror(input));
    int closed = fclose(input);
    assert(closed == 0);
    source[count] = 0;
}
/** Observe owned imported values through generated typed native borrows, never runtime adapter casts. */
static bool inspect_payload(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".inspect") != 0) { return false; }
    int written = fputs("AmaByteView bytes; *status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(0)->payload,&bytes);\n"
        "if(status->code!=AMA_OK) { return; } size_t code;\n"
        "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(1)->payload,&code);\n"
        "if(status->code!=AMA_OK) { return; } printf(\"payload=%zu:%zu:\",code,bytes.length);\n"
        "if(bytes.length!=0) { fwrite(bytes.data,1,bytes.length,stdout); } putchar('\\n');\n", output);
    assert(written != EOF);
    return true;
}
/** Actual methods share resource lifetime while all token/result values remain ordinary declared Ama data. */
static void it_imports_retained_completions_and_rejects_duplicate_and_reused_tokens(void) {
    directory("generated/project");
    char source[16384];
    fixture("service", source);
    native_fixture("inspect", "@<\ndata\n..bytes\ncode\n..integer\n@C\n");
    native_entry_override = inspect_payload;
    execute_project(source, "1:1:0\n2:2:0\n3:3:0\n4:4:0\n5:5:0\n6:6:0\n7:7:0\n8:8:0\npayload=7:6:binary\n", true);
    native_entry_override = NULL;
}
/** Install the maintained typed ownership graph shared by every scheduling scenario. */
static void install_protocol(void) {
    const char *names[] = {"record", "task", "registration", "validate", "mailbox", "import"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char source[16384], path[256];
        fixture(names[index], source);
        int length = snprintf(path, sizeof(path), "generated/project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, source);
    }
}
/** Exercise each completion boundary using the same actual task and declared mailbox graph. */
static void it_reconciles_completion_before_and_after_the_awaiting_recheck(void) {
    for (unsigned timing = 0; timing < 6; timing++) {
        directory("generated/project");
        install_protocol();
        char setup[16384], reconcile[16384], source[32768];
        fixture("setup", setup);
        fixture("reconcile", reconcile);
        const char *post = "service\n post !\n  < token\n  < |7\n  < |complete\n";
        int length = snprintf(source, sizeof(source),
            "%stask& !\n < &record\n < &wait\n < &service\n < token\n < |%u\n < |%u\n < |%u\n%s%s%s%s%s"
            "ready&[item&]\n{\n probe !\n  < |1\n item& %%\n}\n"
            "task&\n phase >> phase\nphase\n completed >> completed\ncompleted\n?\n probe !\n  < |2\n",
            setup, timing == 0, timing == 1, timing == 2, (timing == 3 || timing == 5) ? post : "", timing == 5 ? "task& %\n" : "", reconcile,
            timing == 4 ? post : "", timing == 4 ? reconcile : "");
        assert(length > 0 && (size_t)length < sizeof(source));
        execute_project(source, "1:1:0\n2:2:0\n", true);
    }
}
/** Withdrawn registrations and mismatched full identities must not mutate a mailbox. */
static void it_rejects_revoked_waits_and_mismatched_import_identities(void) {
    const char *changes[] = {"wait\n live !\n", "record\n epoch !\n",
        "record\n generation !\n", "record\n enrolled !\n", "record\n waiting !\n",
        "service\n prepare !\n  > different\nwait !\n < task&\n < epoch\n < generation\n < different\n < mailbox&\n"};
    for (size_t index = 0; index < sizeof(changes) / sizeof(*changes); index++) {
        directory("generated/project");
        install_protocol();
        char setup[16384], source[32768];
        fixture("setup", setup);
        int length = snprintf(source, sizeof(source),
            "%stask& !\n < &record\n < &wait\n < &service\n < token\n < |%u\n < |0\n < |0\n"
            "service\n post !\n  < token\n  < |7\n  < |complete\nservice\n take !\n  > packet\n"
            "record\n epoch >> epoch\n generation >> generation\n%s"
            "import !\n < &wait\n < packet\n > accepted\naccepted\n:\n probe !\n  < |1\n"
            "mailbox&\n delivered >> delivered\ndelivered\n:\n probe !\n  < |2\n"
            "validate !\n < &wait\n > current wait\ncurrent wait\n?\n mailbox&\n  ready >> ready flag\n ready flag\n ?\n"
            "  ready&[]\n   append !\n    < task&\nready&[]\n length !\n  > count\ncount\n:\n probe !\n  < |3\n", setup, index < 5, changes[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        execute_project(source, "1:1:0\n2:2:0\n3:3:0\n", true);
    }
}
/** Invalid lifecycle calls terminate technically and still retire already prepared native resources. */
static void it_rejects_invalid_service_lifecycle_operations_without_leaking_owners(void) {
    const char *cases[] = {
        "@!\nservice\n..task.events\nservice !\n < |0\n",
        "@!\nmethod\n..task.events.prepare\nmethod !\n",
        "@!\nservice\n..task.events\nservice !\n < |1\nservice\n prepare !\n  > token\n"
            "service\n post !\n  < token\n  < |1\n  < |unarmed\n",
        "@!\nservice\n..task.events\nservice !\n < |1\nservice\n prepare !\n  > token\n"
            "service\n arm !\n  < token\nservice\n arm !\n  < token\n",
        "@!\nservice\n..task.events\nservice !\n < |1\nservice\n prepare !\n  > token\n"
            "service\n prepare !\n  > extra\n"};
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        directory("generated/project");
        execute_project(cases[index], "", false);
        if (index == 0) { expect_diagnostic("Invalid native event service creation"); }
    }
}
/** Run actual compiler/library composition rather than a mock dispatcher. */
void run(void) {
    const char *selected = getenv("AMA_EVENT_BRIDGE_CASE");
    if (selected == NULL || strcmp(selected, "service") == 0) { it_imports_retained_completions_and_rejects_duplicate_and_reused_tokens(); }
    if (selected == NULL || strcmp(selected, "reconcile") == 0) { it_reconciles_completion_before_and_after_the_awaiting_recheck(); }
    if (selected == NULL || strcmp(selected, "invalid") == 0) { it_rejects_revoked_waits_and_mismatched_import_identities(); }
    if (selected == NULL || strcmp(selected, "lifecycle") == 0) { it_rejects_invalid_service_lifecycle_operations_without_leaking_owners(); }
    assert(selected == NULL || strcmp(selected, "service") == 0 || strcmp(selected, "reconcile") == 0
        || strcmp(selected, "invalid") == 0 || strcmp(selected, "lifecycle") == 0);
}
/** Return only after every observation matches. */
int main(void) { run(); return 0; }
