/** @file
 * @brief Provide actual three-origin producer source paths for cached candidate compositions.
 */
#ifndef AMA_STATIC_REGISTRY_CACHE_TEST_COMMON
#define AMA_STATIC_REGISTRY_CACHE_TEST_COMMON
#include "../common.c"

/** Static producer edges have explicit named maps; the fixed edges make
 * concrete cached descendants independent of the current owner's unknown input.
 */
static const char *main_source = "@...\n...out\n...element\n"
    ".other\n ...element < ...element\n ...out > ...other result\n"
    ".other\n ...element < ..text\n ...out > ...fixed other result\n"
    ".leaf\n ...element < ..integer\n ...out > ...leaf result\n"
    ".leaf\n ...element < ..integer\n ...out > ...second leaf result\n"
    "...anchor\n..integer\n...text\n..text\n";
static const char *other_source = "@...\n...out\n...element\n"
    ".main\n ...element < ...element\n ...out > ...main result\n"
    ".leaf\n ...element < ..integer\n ...out > ...leaf result\n";
static const char *leaf_source = "@...\n...out\n...element\n"
    ".main\n ...element < ..text\n ...out > ...main result\n";

/** Load three actual graph origins and an independent concrete argument Type. */
static const StaticTerm *setup_producers(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/leaf", leaf_source);
    setup_registry(main_source, other_source);
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "..text"));
    const StaticTerm *type;
    assert(text && static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &type));
    return type;
}

/** Select an exact written projection by its local defining receipt. */
static const AstTypeExpression *projection(const StaticModule *module, const char *receiver) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->receiver && layout_text_is(site->receiver->receiver, receiver)) {
            assert(site->statement->kind == AST_STATIC_PROJECTION);
            return site->statement->as.static_projection;
        }
    }
    assert(false); return NULL;
}

/** Bind the producer's full one-input identity before selecting a compilation root. */
static const StaticCreationPath *producer_root(const StaticModule *module, const StaticTerm *value) {
    StaticTermCapture input = {.formal = application_formal(module, 0), .value = value};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, module, &input, 1, &environment));
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, environment, &path));
    return path;
}

/** Inputs are resolved fixture IR for the selected source binding, not a
 * replacement for T211's graph equation evaluator. No output is evaluated here.
 */
static const StaticCreationPath *producer_child(const StaticCreationPath *parent,
    const char *receiver, const StaticModule *target, const StaticTerm *value) {
    StaticTermCapture input = {.formal = application_formal(target, 0), .value = value};
    const StaticCreationPath *path;
    StaticCreationOutcome outcome;
    assert(static_creation_producer_enter(&guards, parent,
        projection(parent->owner->environment->definition, receiver), &input, 1, &path, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED);
    return path;
}

#endif
