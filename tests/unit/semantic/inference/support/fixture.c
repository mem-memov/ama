/** @file
 * @brief Supply real typed modules in separate root families and own every inference stage result.
 */
#include "semantic/internal.h"
/** Prepare the explicit supported native metadata and ordinary result interfaces, without executing C. */
static void prepare_inference(void) {
    prepare_types();
    directory("generated/standard");
    const char *names[] = {"integer", "value"};
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t name = 0; name < 2; name++) {
        for (size_t suffix = 0; suffix < 4; suffix++) {
            char path[128];
            int length = snprintf(path, sizeof(path), "generated/standard/%s%s", names[name], suffixes[suffix]);
            assert(length > 0 && (size_t)length < sizeof(path));
            file(path, suffix == 0 ? "@C\n" : suffix == 3
                ? "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] position\n" : "/* native metadata fixture */\n");
        }
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\nresult\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\nvalues[]\n/test/fixture/v1.0.0/src/value\nreferences&[]\n/test/fixture/v1.0.0/src/value\n@!\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/palette", "@=\ntone\n/test/fixture/v1.0.0/src/value\n |1\ntones[]\n/test/fixture/v1.0.0/src/value\n |1\n");
}
/** Parse complete roots, build explicit/native inventories and run inference once; retain failed owners for assertions. */
static SemanticStatus infer_text(const char *text) {
    file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
    setup_semantic();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main");
    if (loaded != PROJECT_OK) { fprintf(stderr, "Inference load: %s\n%s", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_names(&semantic);
    if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
    if (status == SEMANTIC_OK) { status = semantic_infer(&semantic); }
    return status;
}
/** Find a module field by exact immutable test spelling and assert that no substring/suffix lookup is used. */
static SemanticSymbol *field(const char *name) {
    size_t length = strlen(name);
    SemanticType *entry = semantic_find_type(&semantic,"/test/fixture/v1.0.0/src/main");
    if (!entry) { return NULL; }
    for (SemanticSymbol *symbol = entry->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->name.end - symbol->name.start == length
            && memcmp(source_bytes(symbol->name.source) + symbol->name.start, name, length) == 0) { return symbol; }
    }
    return NULL;
}
/** Require a successful structural/type pass and print its actual source diagnostic on regression. */
static void valid_inference(const char *text) {
    SemanticStatus status = infer_text(text);
    if (status != SEMANTIC_OK) { fprintf(stderr, "Inference: %s\n%s", semantic.error.message, text); }
    assert(status == SEMANTIC_OK && semantic.inference_valid);
}
