/** @file
 * @brief Resolve explicit builtin transport profiles and bounded list dependencies.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) rejects private
 * payload serialization and reference transport without a reconstruction policy.
 */
#include <string.h>
#include "../internal.h"
/** Check the declared native value hooks needed by a fixed transport profile. */
static bool native_contract(const SemanticType *type, bool literal) {
    return type != NULL && type->module->syntax.sections.kind == MODULE_NATIVE
        && type->cloneable && type->abi.bindings[ABI_CLONE].present
        && type->abi.bindings[ABI_CONDITION].present
        && (!literal || type->abi.bindings[ABI_LITERAL].present);
}
/** Require the selected fixed profile's nominal identity and complete native capability contract. */
static bool scalar_schema(LibraryBuild *build, LibrarySchema *schema, const SemanticProgram *program) {
    static const char *const paths[] = {"..integer", "..text", "..std.condition", "..failure"};
    if (strcmp(schema->argument, paths[schema->kind]) != 0) {
        return library_fail(build, schema->site, "The builtin transport profile requires its exact standard module identity");
    }
    schema->type = semantic_find_type(program, schema->argument);
    if (!native_contract(schema->type, schema->kind != LIBRARY_FAILURE)) {
        return library_fail(build, schema->site, "The declared native transport contract is unavailable or lacks its required capabilities");
    }
    if (schema->kind == LIBRARY_FAILURE) {
        const SemanticSymbol *description = schema->type->first;
        if (description == NULL || description->next != NULL || description->role != SYMBOL_RESULT
            || description->reference || description->list
            || description->name.end - description->name.start != sizeof("description") - 1
            || memcmp(source_bytes(description->name.source) + description->name.start, "description", sizeof("description") - 1) != 0
            || strcmp(description->type->module->source->logical_path, "..text") != 0
            || !native_contract(description->type, true)) {
            return library_fail(build, schema->site, "The failure transport requires one owned text description result");
        }
    }
    return true;
}
bool library_resolve_schemas(LibraryBuild *build, const SemanticProgram *program) {
    for (LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (schema->kind != LIBRARY_LIST && !scalar_schema(build, schema, program)) { return false; }
    }
    for (LibrarySchema *schema = build->schemas; schema != NULL; schema = schema->next) {
        if (schema->kind != LIBRARY_LIST) { continue; }
        schema->element = library_schema_find(build, schema->argument);
        if (schema->element == NULL || schema->element->kind == LIBRARY_LIST) {
            return library_fail(build, schema->site, "A list transport needs an explicit supported scalar element schema");
        }
        schema->type = schema->element->type;
    }
    return true;
}
