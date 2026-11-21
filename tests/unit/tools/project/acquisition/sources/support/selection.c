/** @file
 * @brief Compose actual package aliases and finite selector unions with scoped source planning.
 * @see [T234 contract](../../../../../../../docs/implementation/acquisition-plan.md)
 */
#include "source/configuration/selection/selection.h"
/** Retain every actual wildcard member once, skipping missing/unsuffixed members
 * and selecting each candidate's most specific route in the requesting scope. */
static void it_routes_every_actual_alias_mask_match_once_in_either_inventory_order(void) {
    static const char *const names[] = {
        "/client/root/v1.0.0", "/a/p/v2.4.14-speed", "/a/p/v2.4.14-speed/src/mod1",
        "/a/algorithms/q/v2.4.14-memory", "/a/algorithms/q/v2.4.14-memory/src/mod1",
        "/a/p/v2.4.14", "/a/p/v2.4.14/src/mod1", "/a/empty/v2.4.14-speed"
    };
    for (int reversed = 0; reversed < 2; reversed++) {
        begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
        ConfigurationDocument owner;
        ConfigurationStatus parsed = document(&owner,
            "[aliases]\n~tree = /a\n[dependencies]\n~tree/*/v2.4.14-*/src/mod1\n"
            "/a/p/v2.4.14-speed/src/mod1\n/a/*/v2.4.15-*/src/mod1\n"
            "[repository /a]\nurl = broad\ndirectory = tree\nrevision = main\n"
            "[repository /a/algorithms/q]\nurl = specific\ndirectory = selected\nrevision = main\n", CONFIGURATION_PACKAGE);
        assert(parsed == CONFIGURATION_OK);
        const ModulePathValue *declaring = identity(names[0]);
        const ConfigurationAliasScope *aliases = NULL;
        ConfigurationScopeStatus scoped = configuration_package_aliases(&metadata, &owner,
            (const unsigned char *)declaring->text, declaring->length, &aliases, &issue);
        assert(scoped == CONFIGURATION_SCOPE_OK);
        const DependencySelector *patterns[3]; size_t count = 0;
        for (const ConfigurationSelector *entry = owner.selectors; entry; entry = entry->next) {
            assert(count < 3);
            DependencySelectorStatus compiled = dependency_selector_compile(&metadata, entry->pattern, aliases, &patterns[count], &issue);
            assert(compiled == DEPENDENCY_SELECTOR_OK); count++;
        }
        assert(count == 3);
        DependencyViewInput entries[8];
        for (size_t index = 0; index < 8; index++) {
            entries[index] = (DependencyViewInput){identity(names[reversed ? 7 - index : index]), 0};
        }
        const DependencyView *view = NULL;
        DependencySelectionStatus status = dependency_view_build(&results, entries, 8, &view, &issue); assert(status == DEPENDENCY_SELECTION_OK);
        const DependencySelection *selection = NULL;
        status = dependency_selection_build(&results, view, declaring, patterns, count, &selection, &issue);
        assert(status == DEPENDENCY_SELECTION_OK && selection->count == 2);
        bool broad = false, specific = false; size_t visited = 0;
        for (const DependencySelectionRoot *root = selection->roots; root; root = root->next) {
            AcquisitionSourceRequest value = {0};
            AcquisitionSourcesStatus planned = acquisition_sources_request(&plan, &owner, root->entry->identity, &value, &problem);
            assert(planned == ACQUISITION_SOURCES_OK && value.resolve_snapshot && !value.local);
            resolve(&value, commit_a);
            const AcquisitionVersionSource *bound = NULL;
            planned = acquisition_sources_bind(&plan, &value, &bound, &problem); assert(planned == ACQUISITION_SOURCES_OK);
            if (!strcmp(bound->snapshot->url, "broad")) {
                assert(!broad && !strcmp(bound->version, "/a/p/v2.4.14-speed") && !strcmp(bound->directory, "tree/p/v2.4.14-speed")); broad = true;
            } else {
                assert(!specific && !strcmp(bound->snapshot->url, "specific"));
                assert(!strcmp(bound->version, "/a/algorithms/q/v2.4.14-memory") && !strcmp(bound->directory, "selected/v2.4.14-memory")); specific = true;
            }
            visited++;
        }
        assert(visited == 2 && broad && specific && plan.snapshot_count == 2 && plan.version_count == 2);
        const DependencySelection *empty = NULL;
        status = dependency_selection_build(&results, view, declaring, &patterns[2], 1, &empty, &issue);
        assert(status == DEPENDENCY_SELECTION_OK && !empty->count && !empty->roots);
        acquisition_sources_destroy(&plan); configuration_document_destroy(&owner); finish();
    }
}
