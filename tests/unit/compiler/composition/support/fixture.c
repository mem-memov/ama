/** @file
 * @brief Supply complete current-language projects whose native C cannot be compiled or executed during analysis.
 */
#include "semantic/specializations/inventory.h"

static const char composed_source[] = "@=\nbase\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/seed\n@<\nexternal&\n/test/fixture/v1.0.0/src/value\n@>\nself&\n/test/fixture/v1.0.0/src/main\n &\n@!\n"
    "producer\n/test/fixture/v1.0.0/src/producer\nsource\n/test/fixture/v1.0.0/src/value\n |literal % ref& # bytes  \nitems[]\n/test/fixture/v1.0.0/src/value\n |1\n"
    "producer !\n < source\n method !\n  > received\n  next& %\n   > resumed\n  :\n   %\n"
    "items[item&]\n{\n saved&\n  item&\n items[]\n  append !\n   < &source\n   > created&\n %\n}\n"
    "items[]\n length !\n  value\n  {\n  }\ncontinued\n % source\n";
/** Keep project/standard identities distinct and all native contracts explicit. */
static void prepare_composed_files(const char *text) {
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/standard");
    const char *roots[] = {"generated/project/packages/test/fixture/v1.0.0/src/value", "generated/standard/integer"};
    for (size_t index = 0; index < 2; index++) {
        char path[128];
        const char *suffixes[] = {"", ".h", ".c", ".abi"};
        for (size_t part = 0; part < 4; part++) {
            int size = snprintf(path, sizeof(path), "%s%s", roots[index], suffixes[part]);
            assert(size > 0 && (size_t)size < sizeof(path));
            file(path, part == 0 ? "@C\n" : part == 3
                ? "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n"
                : "#error Frontend composition must not compile or execute native C\n");
        }
    }
    file("generated/project/packages/test/fixture/v1.0.0/src/producer", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\nmethod\n/test/fixture/v1.0.0/src/worker\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/worker", "@>\nvalue\n/test/fixture/v1.0.0/src/value\nnext&\n/test/fixture/v1.0.0/src/worker\n@!\n%\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nseed\n/test/fixture/v1.0.0/src/value\n |7\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unused draft", "@>\nunfinished\n/test/fixture/v1.0.0/src/missing dependency\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", text);
}
/** Check the exact same two configured root families through the normal public API. */
static bool check_composed(void) {
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    return compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main");
}
/** Canonical modules and original half-open source ranges define equality across independent semantic owners sharing one Project. */
static bool same_type(const SemanticType *left, const SemanticType *right) {
    return left == NULL || right == NULL ? left == right : left->module == right->module;
}
/** Source records borrow the exact same loaded source owner; no reconstructed line/column or digest identity is used. */
static bool same_span(SourceSpan left, SourceSpan right) {
    return left.source == right.source && left.start == right.start && left.end == right.end;
}
/** Run the documented six stages explicitly, preserving the exact first failed prefix. */
static SemanticStatus staged_semantics(SemanticProgram *program, StaticClosurePlan *closure, const SemanticNativeAuthoring *authoring) {
    SemanticStatus status = semantic_specialization_names(program,closure);
    if (status == SEMANTIC_OK) { status = semantic_native_authoring(program, authoring); }
    if (status == SEMANTIC_OK) { status = semantic_infer(program); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(program); }
    if (status == SEMANTIC_OK) { status = semantic_relations(program); }
    if (status == SEMANTIC_OK) { status = semantic_constants(program); }
    return status;
}
