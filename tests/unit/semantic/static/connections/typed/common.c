/** @file
 * @brief Prepare symbolic owner environments and original runtime field identities for typed-scope checks.
 */
#ifndef AMA_STATIC_TYPED_SCOPE_TEST_COMMON
#define AMA_STATIC_TYPED_SCOPE_TEST_COMMON
#include "../common.c"
#include "semantic/static/constraints/internal.h"

/** A source-generic owner has explicit, inferred and foreach-local Type terms. */
static const char *typed_source = "@<\ninput\n...slot\n@!\nknown\n..integer\ntext anchor&\n..text\n"
    "worker\n.other\nworker !\n > fresh\nvalues[]\n...slot\nvalues[item&]\n{\n borrowed&\n  item&\n}\n";

/** Unknown result Type belongs to the worker template, not a guessed native Type. */
static void setup_typed(void) {
    setup_connections(typed_source, "@>\nvalue\n...result\n@!\n");
}

/** Only an explicitly selected root supplies a generic actual-context provenance. */
static const StaticResultContext *typed_actual(void) { return publication_context(caller_environment); }

/** Metadata-only nominal environments do not register actual contexts/roots. */
static const StaticTermEnvironment *typed_environment(const StaticTerm *slot) {
    StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = slot};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    return environment;
}

/** Query by the already fixed source binding, never infer another runtime name. */
static const StaticTypedField *typed_field(const StaticTypedScope *scope, const char *name) {
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, name));
    assert(definition != NULL);
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope, definition, &field));
    return field;
}
#endif
