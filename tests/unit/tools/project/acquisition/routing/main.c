/** @file
 * @brief Exhaust bounded requester-scoped routing independently of production prefix scans.
 * @see [T234 contract](../../../../../../docs/implementation/acquisition-plan.md)
 */
#include "../common.c"
#include "source/configuration/aliases/aliases.h"
static const char *const prefixes[] = {"/a", "/a/p", "/a/p/q", "/a/pp", "/b"};
static const char *const requests[] = {
    "/a/p/q/v1.0.0/src/m", "/a/p/v1.0.0/tests/t", "/a/pp/v1.0.0/src/m",
    "/b/p/v1.0.0/src/m", "/ab/p/v1.0.0/src/m", "/a/p/q/v1.0.0"
};
/* Independent finite relation, listed in descending whole-component priority. */
static const int expected[][4] = {{2, 1, 0, -1}, {1, 0, -1, -1}, {3, 0, -1, -1},
    {4, -1, -1, -1}, {-1, -1, -1, -1}, {2, 1, 0, -1}};
static size_t model_tables;
/** Check a complete table against the explicit relation, including empty/no-route cases. */
static void table(const size_t *order, size_t count, unsigned members) {
    begin(); model_tables++;
    char text[2048] = {0}; size_t used = 0;
    for (size_t index = 0; index < count; index++) {
        size_t id = order[index];
        int added = snprintf(text + used, sizeof(text) - used,
            "[repository %s]\nurl = remote%zu\ndirectory = tree%zu\nrevision = branch%zu\n", prefixes[id], id, id, id);
        assert(added > 0 && (size_t)added < sizeof(text) - used); used += (size_t)added;
    }
    ConfigurationDocument scope;
    ConfigurationStatus parsed = document(&scope, text, CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
    for (size_t query = 0; query < sizeof(requests) / sizeof(requests[0]); query++) {
        const ModulePathValue *path = identity(requests[query]);
        int wanted = -1;
        for (size_t rank = 0; rank < 4 && expected[query][rank] >= 0; rank++) {
            int candidate = expected[query][rank];
            if (members & (1u << candidate)) { wanted = candidate; break; }
        }
        AcquisitionRoute route = {.scope = &scope, .prefix_length = 999};
        size_t allocations_before = allocation_count;
        AcquisitionRouteStatus status = acquisition_route_select(&scope, path, &route, &issue);
        assert(allocation_count == allocations_before);
        if (wanted < 0) {
            assert(status == ACQUISITION_ROUTE_MISSING && !route.mapping && route.prefix_length == 999);
            assert(issue.span.source == path->expression.source && issue.span.start == path->expression.start);
        } else {
            char name[32]; (void)snprintf(name, sizeof(name), "remote%d", wanted);
            assert(status == ACQUISITION_ROUTE_OK && route.scope == &scope && route.request == path);
            assert(equal(route.mapping->url, name) && route.prefix_length == strlen(prefixes[wanted]));
        }
    }
    configuration_document_destroy(&scope); finish();
}
/** Enumerate every ordered subset of five mappings; depth is bounded by that domain. */
static void ordered_subsets(size_t *order, size_t count, unsigned members) {
    table(order, count, members);
    for (size_t index = 0; index < 5; index++) {
        if (members & (1u << index)) { continue; }
        order[count] = index;
        ordered_subsets(order, count + 1, members | (1u << index));
    }
}
/** All 326 ordered subsets and six requests must agree with the finite relation. */
static void it_matches_every_bounded_reordered_scope_without_character_prefix_leakage(void) {
    size_t order[5]; model_tables = 0;
    ordered_subsets(order, 0, 0);
    assert(model_tables == 326);
}
/** A target or sibling scope cannot replace the requesting scope or trigger fallback. */
static void it_retains_requesting_scope_and_selects_unavailable_specific_routes(void) {
    begin();
    ConfigurationDocument first, target;
    ConfigurationStatus parsed = document(&first,
        "[repository /a]\nurl = broad\ndirectory = .\n"
        "[repository /a/p]\nurl = nonexistent-specific-source\ndirectory = narrow\n", CONFIGURATION_PACKAGE);
    assert(parsed == CONFIGURATION_OK);
    parsed = document(&target, "[repository /a/p]\nurl = unrelated-target-scope\ndirectory = other\n", CONFIGURATION_PACKAGE);
    assert(parsed == CONFIGURATION_OK);
    const ModulePathValue *path = identity("/a/p/v2.4.14-speed/src/m");
    AcquisitionRoute route, again;
    AcquisitionRouteStatus status = acquisition_route_select(&first, path, &route, &issue);
    assert(status == ACQUISITION_ROUTE_OK && route.scope == &first && equal(route.mapping->url, "nonexistent-specific-source"));
    status = acquisition_route_select(&target, path, &again, &issue);
    assert(status == ACQUISITION_ROUTE_OK && again.scope == &target && equal(again.mapping->url, "unrelated-target-scope"));
    status = acquisition_route_select(&first, path, &again, &issue);
    assert(status == ACQUISITION_ROUTE_OK && again.mapping == route.mapping && again.scope == &first);
    configuration_document_destroy(&target); configuration_document_destroy(&first); finish();
}
/** Invalid profiles/endpoints preserve outputs and original diagnostic provenance. */
static void it_refuses_invalid_inputs_without_publishing_a_partial_route(void) {
    begin();
    ConfigurationDocument scope, module, malformed;
    ConfigurationStatus parsed = document(&scope, "", CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
    parsed = document(&module, "", CONFIGURATION_MODULE); assert(parsed == CONFIGURATION_OK);
    parsed = document(&malformed, "[repository /a]\nurl = incomplete\n", CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_INVALID);
    const char *invalid[] = {"//integer", "/a/p", "/a/p/v1.0.0/src"};
    AcquisitionRoute output = {.scope = &scope, .prefix_length = 123};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        const ModulePathValue *path = identity(invalid[index]);
        AcquisitionRouteStatus status = acquisition_route_select(&scope, path, &output, &issue);
        assert(status == ACQUISITION_ROUTE_INVALID && output.scope == &scope && output.prefix_length == 123 && !output.mapping);
        assert(issue.span.source == path->expression.source);
    }
    const ModulePathValue *root = NULL;
    ModulePathStatus canonical = module_path_from_canonical(&metadata, (const unsigned char *)"/", 1, &root, &issue);
    assert(canonical == MODULE_PATH_OK);
    AcquisitionRouteStatus root_status = acquisition_route_select(&scope, root, &output, &issue);
    assert(root_status == ACQUISITION_ROUTE_INVALID && output.prefix_length == 123 && !issue.span.source);
    const ModulePathValue *path = identity("/a/p/v1.0.0/src/m");
    const ConfigurationDocument *bad[] = {NULL, &module, &malformed};
    for (size_t index = 0; index < sizeof(bad) / sizeof(bad[0]); index++) {
        AcquisitionRouteStatus status = acquisition_route_select(bad[index], path, &output, &issue);
        assert(status == ACQUISITION_ROUTE_INVALID && output.prefix_length == 123);
    }
    AcquisitionRouteStatus status = acquisition_route_select(&scope, NULL, &output, &issue); assert(status == ACQUISITION_ROUTE_INVALID && !issue.span.source);
    status = acquisition_route_select(&scope, path, NULL, &issue); assert(status == ACQUISITION_ROUTE_INVALID);
    configuration_document_destroy(&malformed); configuration_document_destroy(&module); configuration_document_destroy(&scope); finish();
}
/** Normalized Unicode aliases retain the original bounded caller expression. */
static void it_routes_unicode_alias_values_while_preserving_original_expression_offsets(void) {
    begin();
    ConfigurationDocument owner, empty;
    ConfigurationStatus parsed = document(&owner,
        "[aliases]\n~source = /厂/包/v1.0.0\n"
        "[repository /厂]\nurl = broad\ndirectory = 根\n"
        "[repository /厂/包]\nurl = exact\ndirectory = 目录\n"
        "[repository /厂/包裹]\nurl = wrong\ndirectory = other\n", CONFIGURATION_PACKAGE);
    assert(parsed == CONFIGURATION_OK);
    parsed = document(&empty, "", CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
    const ConfigurationAliasScope *aliases = NULL;
    const char *version = "/caller/p/v1.0.0";
    ConfigurationScopeStatus scoped = configuration_package_aliases(&metadata, &owner,
        (const unsigned char *)version, strlen(version), &aliases, &issue);
    assert(scoped == CONFIGURATION_SCOPE_OK);
    SourceSpan expression = configuration_test_text("prefix\n~source/src/m suffix");
    expression.start = 7; expression.end = 7 + strlen("~source/src/m");
    const ModulePathValue *path = NULL;
    ModulePathStatus normalized = module_path_normalize(&metadata, expression, NULL,
        configuration_alias_lookup, (void *)aliases, &path, &issue);
    assert(normalized == MODULE_PATH_OK && !strcmp(path->text, "/厂/包/v1.0.0/src/m"));
    AcquisitionRoute output;
    AcquisitionRouteStatus status = acquisition_route_select(&owner, path, &output, &issue);
    assert(status == ACQUISITION_ROUTE_OK && equal(output.mapping->url, "exact") && output.prefix_length == strlen("/厂/包"));
    status = acquisition_route_select(&empty, path, &output, &issue);
    assert(status == ACQUISITION_ROUTE_MISSING && issue.span.source == expression.source && issue.span.start == 7 && issue.span.end == expression.end);
    configuration_document_destroy(&empty); configuration_document_destroy(&owner); finish();
}
/** Execute finite-model, scope and refusal partitions explicitly. */
void run(void) {
    it_matches_every_bounded_reordered_scope_without_character_prefix_leakage();
    it_retains_requesting_scope_and_selects_unavailable_specific_routes();
    it_refuses_invalid_inputs_without_publishing_a_partial_route();
    it_routes_unicode_alias_values_while_preserving_original_expression_offsets();
}
/** Require all borrowed source/arena owners to retire after the assertions. */
int main(void) { run(); return 0; }
