/** @file
 * @brief Supply complete multi-module projects with current constructs and native bodies that semantic checking must not compile or execute.
 */
static const char composed_main[] =
    "@=\nbase\n.value\n .palette.seed\n@<\ninitial&\n.value\n@>\nfailure\n.value\n .palette.seed\nself&\n.main\n &\n@!\n"
    "source\n.value\n |data % ref& # bytes  \nitems[]\n.value\n |1\nproducer\n.producer\n"
    "producer\n method !\n  < source\n  > received\n method %\n  > resumed\n"
    "items[]\n append !\n  < &source\n  > created&\n length !\n  > count\n"
    "items[item&]\n{\n saved&\n  item&\n %\n item& !\n}\n"
    "producer !\n method %\n  condition\n  {\n   ^\n  }\n"
    "source\n:\n fallback\n  source\ncontinued\n % source\n";
/** Prepare the exact root/native declarations; #error companions make accidental C compilation observable. */
static void prepare_composition(void) {
    prepare_types();
    file("generated/program/value.h", "#error Semantics must not compile native headers\n");
    file("generated/program/value.c", "#error Semantics must not execute native constructors\n");
    file("generated/program/producer", "@>\nmethod\n.worker\n@!\n");
    file("generated/program/worker", "@<\ninput\n.value\nexternal&\n.resource\n@>\noutput\n.value\ncondition\n.value\n@!\n"
        "state\n.value\n |2\noutput\n state\ncondition\n state\n%\n");
    file("generated/program/palette", "@=\nseed\n.value\n |7\n");
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.h", "#error Semantics must not inspect the size bridge C declaration\n");
    file("generated/standard/integer.c", "#error Semantics must not invoke native C\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
}
/** Own one real two-root source closure without pre-running namespace construction. */
static void load_composition(const char *text) {
    file("generated/program/main", text);
    setup_project();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus status = project_load(&project, roots, 2, ".main");
    if (status != PROJECT_OK) { fprintf(stderr, "Composed fixture: %s\n%s\n", project.error.message, text); }
    assert(status == PROJECT_OK);
}
/** Execute the explicit documented stage sequence, stopping before any consumer of failed output. */
static SemanticStatus analyze_staged(SemanticProgram *program) {
    SemanticStatus status = semantic_names(program);
    if (status == SEMANTIC_OK) { status = semantic_native(program); }
    if (status == SEMANTIC_OK) { status = semantic_infer(program); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(program); }
    if (status == SEMANTIC_OK) { status = semantic_relations(program); }
    if (status == SEMANTIC_OK) { status = semantic_constants(program); }
    return status;
}
/** Exact nominal comparison across fresh semantic owners sharing one immutable loaded project. */
static bool same_type(const SemanticType *left, const SemanticType *right) {
    return left == NULL || right == NULL ? left == right : left->module == right->module;
}
/** Original source identity and complete half-open byte ranges define source metadata equality. */
static bool same_span(SourceSpan left, SourceSpan right) {
    return left.source == right.source && left.start == right.start && left.end == right.end;
}
