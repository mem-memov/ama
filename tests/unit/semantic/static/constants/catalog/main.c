/** @file
 * @brief Verify qualified source ownership, exact loaded provenance and inventory failure boundaries.
 */
#include "../../common.c"
#include "semantic/static/events/internal.h"
#include "semantic/static/classify/sites.h"
#include <stdint.h>

/** Real parser/project shapes include every source-owning node and nested block. */
static const char *source_text = "@=\nshared\n/test/fixture/v1.0.0/src/other\n /test/fixture/v1.0.0/src/other/value\n"
    "@<\nplaceholder\n.other.value\ninput\n/test/fixture/v1.0.0/src/other\n /test/fixture/v1.0.0/src/other/value\n"
    "@>\noutput\n/test/fixture/v1.0.0/src/other\n /test/fixture/v1.0.0/src/other/items[]\n@!\nlocal\n/test/fixture/v1.0.0/src/other\n /test/fixture/v1.0.0/src/other/value\n"
    "local\n /test/fixture/v1.0.0/src/other/value\nworker !\n < /test/fixture/v1.0.0/src/other/value\n:\n handler\n /test/fixture/v1.0.0/src/other\n  /test/fixture/v1.0.0/src/other/value\n"
    "flag\n?\n positive\n /test/fixture/v1.0.0/src/other\n  /test/fixture/v1.0.0/src/other/value\n:\n negative\n /test/fixture/v1.0.0/src/other\n  /test/fixture/v1.0.0/src/other/value\n"
    "items[item&]\n{\n entry\n /test/fixture/v1.0.0/src/other\n  /test/fixture/v1.0.0/src/other/value\n}\n";

/** Each source is cataloged once without synthetic type heads. Original spans
 * select exact nonimplicit dependency requests; same text at another site fails.
 */
static void it_collects_all_original_sources_and_exact_loaded_dependency_occurrences(void) {
    setup_static(source_text, "@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->constant_source_count == 10 && module->parameter_count == 1);
    assert(layout_text_is(module->first->name, ".other.value"));
    size_t count = 0, lists = 0;
    for (const StaticConstantOccurrence *site = module->first_constant_source; site; site = site->next) {
        assert(site->source->kind == AST_CONSTANT_SOURCE && site->statement != NULL);
        assert(site->origin == project_find(&project, "/test/fixture/v1.0.0/src/other"));
        assert(static_loaded_origin(module, site->source->qualified_path) == site->origin);
        SourceSpan wrong = site->source->qualified_path;
        wrong.end--;
        assert(static_loaded_origin(module, wrong) == NULL);
        assert(site->path != NULL && strcmp(site->path->module->text, "/test/fixture/v1.0.0/src/other") == 0);
        lists += site->source->constant_list;
        count++;
        if (site->next == NULL) { assert(site == module->last_constant_source); }
    }
    assert(count == 10 && lists == 1);
    assert(static_symbol_find(module, static_name_site(project.entry, ".other.value"))->role == STATIC_PARAMETER);
    destroy_static();
}

/** Every collection allocation is refused once; the count overflow cutpoint
 * refuses before linking a source. Failed inventory remains singly owned.
 */
static void it_unwinds_every_qualified_inventory_allocation_and_counter_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_static(source_text, "@!\n");
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = static_collect_occurrences(&inventory, module);
        if (!cut) { assert(success); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && diagnostic_valid(&inventory.error)); }
        failure_at = 0;
        destroy_static();
    }
    setup_static(source_text, "@!\n");
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL);
    module->constant_source_count = SIZE_MAX;
    size_t before = allocation_count;
    assert(!static_constant_occurrences(&inventory, module, project.entry->syntax.constants.first));
    assert(module->first_constant_source == NULL && inventory.status == STATIC_OUT_OF_MEMORY && allocation_count == before);
    destroy_static();
}

/** Qualified catalog binding rejects a missing exact dependency even when an
 * identical namespace is loaded by other original occurrences.
 */
static void it_rejects_missing_qualified_request_provenance_without_textual_fallback(void) {
    setup_static(source_text, "@!\n");
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL && static_collect_explicit(&inventory, module) && static_collect_occurrences(&inventory, module));
    static_sort_occurrences(module);
    assert(static_classify_names(&inventory, module));
    SourceSpan site = module->first_constant_source->source->qualified_path;
    for (ProjectDependency *edge = project.entry->first; edge; edge = edge->next) {
        if (edge->requirement.source == site.source && edge->requirement.start == site.start && edge->requirement.end == site.end) {
            edge->implicit = true;
        }
    }
    assert(!static_bind_occurrences(&inventory, module) && inventory.status == STATIC_INVALID);
    assert(inventory.error.primary.span.start == site.start && diagnostic_valid(&inventory.error));
    destroy_static();
}

/** Run independent collection, exact provenance and resource-boundary checks. */
static void run(void) {
    it_collects_all_original_sources_and_exact_loaded_dependency_occurrences();
    it_unwinds_every_qualified_inventory_allocation_and_counter_refusal();
    it_rejects_missing_qualified_request_provenance_without_textual_fallback();
}
/** Return only after all assertions and counted ownership teardown. */
int main(void) { run(); return 0; }
