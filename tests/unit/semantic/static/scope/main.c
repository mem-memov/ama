/** @file
 * @brief Verify whole-module static identity, public counters, duplicates and allocation ownership.
 */
#include "../common.c"
#include <stdint.h>

/** Equal complete spellings match within their explicit module, while case,
 * suffixes and defining modules remain independent. No lookup allocates.
 */
static void it_keeps_exact_module_scoped_static_identities_and_global_alias_lookup(void) {
    setup_static(STATIC_SCOPE_SOURCE_TEXT, STATIC_SCOPE_SOURCE_TEXT);
    const ProjectModule *main = project.entry, *other = project_find(&project, ".other");
    StaticModule *left = static_module_add(&inventory, main), *right = static_module_add(&inventory, other);
    assert(left != NULL && right != NULL && left != right);
    size_t before = allocation_count;
    assert(static_module_add(&inventory, main) == left && allocation_count == before);
    SourceSpan name = static_name_site(main, "...same"), result = static_name_site(main, "...result");
    SourceSpan alias = static_name_site(main, "...alias"), foreign = static_name_site(other, "...same");
    StaticSymbol *parameter = static_symbol_add(&inventory, left, name, name, STATIC_PARAMETER, STATIC_CATEGORY_TYPE);
    StaticSymbol *output = static_symbol_add(&inventory, left, result, result, STATIC_RESULT, STATIC_CATEGORY_DEFERRED);
    StaticSymbol *received = static_symbol_add(&inventory, left, alias, alias, STATIC_ALIAS, STATIC_CATEGORY_DEFERRED);
    StaticSymbol *independent = static_symbol_add(&inventory, right, foreign, foreign, STATIC_PARAMETER, STATIC_CATEGORY_TYPE);
    assert(parameter != NULL && output != NULL && received != NULL && independent != NULL);
    assert(parameter != independent && parameter->owner != independent->owner);
    assert(left->parameter_count == 1 && left->result_count == 2 && received->category == STATIC_CATEGORY_DEFERRED);
    assert(left->first == parameter && left->last == received && received->next == NULL);
    before = allocation_count;
    assert(static_module_find(&inventory, main) == left && static_module_find(&inventory, other) == right);
    assert(static_symbol_find(left, foreign) == parameter);
    assert(static_symbol_find(right, name) == independent);
    assert(static_symbol_find(left, result) == output);
    assert(static_symbol_find(left, alias) == received);
    assert(static_symbol_find(left, static_name_site(main, "...Same")) == NULL);
    assert(static_symbol_find(left, static_name_site(main, "...same-long")) == NULL);
    assert(static_symbol_find(NULL, name) == NULL && allocation_count == before);
    assert(!static_same_name(static_name_site(main, "...café"), static_name_site(main, "...cafe\xcc\x81")));
    assert(parameter->ordinal == SIZE_MAX && output->ordinal == SIZE_MAX && !inventory.valid);
    destroy_static();
}

/** No combination of roles can redeclare the same local name.
 * Duplicate failure keeps the chain/counts and first
 * source diagnostic unchanged, with no reporting allocation.
 */
static void it_rejects_every_duplicate_role_pair_without_mutation_or_diagnostic_allocation(void) {
    for (StaticRole first = STATIC_PARAMETER; first <= STATIC_ALIAS; first++) {
        for (StaticRole second = STATIC_PARAMETER; second <= STATIC_ALIAS; second++) {
            setup_static(STATIC_SCOPE_SOURCE_TEXT, STATIC_SCOPE_SOURCE_TEXT);
            StaticModule *module = static_module_add(&inventory, project.entry);
            SourceSpan name = static_name_site(project.entry, "...same");
            StaticSymbol *original = static_symbol_add(&inventory, module, name, name, first, STATIC_CATEGORY_TYPE);
            assert(original != NULL);
            SourceSpan later = static_name_site(project.entry, "...result");
            size_t before = allocation_count, parameters = module->parameter_count, results = module->result_count;
            assert(static_symbol_add(&inventory, module, name, later, second, STATIC_CATEGORY_TYPE) == NULL);
            assert(inventory.status == STATIC_INVALID && !inventory.valid && diagnostic_valid(&inventory.error));
            assert(inventory.error.related_count == 1 && inventory.error.related[0].location.span.start == name.start);
            assert(inventory.error.primary.span.start == later.start && allocation_count == before);
            const char *message = inventory.error.message;
            assert(!static_memory_failure(&inventory) && inventory.status == STATIC_INVALID && inventory.error.message == message);
            assert(module->first == original && module->last == original && original->next == NULL);
            assert(module->parameter_count == parameters && module->result_count == results);
            destroy_static();
        }
    }
}

/** Every allocation in insertion is refused once after fixture preparation;
 * partial owners remain destructible and no usable inventory is published.
 */
static void it_unwinds_every_static_record_allocation_failure_and_retains_the_first_error(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(STATIC_SCOPE_SOURCE_TEXT, STATIC_SCOPE_SOURCE_TEXT);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        StaticModule *module = static_module_add(&inventory, project.entry);
        SourceSpan name = static_name_site(project.entry, "...ключ");
        StaticSymbol *symbol = module == NULL ? NULL
            : static_symbol_add(&inventory, module, name, name, STATIC_PARAMETER, STATIC_CATEGORY_TYPE);
        if (failure == 0) {
            assert(symbol != NULL);
            total = allocation_count - before;
            assert(total != 0);
        } else {
            assert(symbol == NULL && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
            assert(diagnostic_valid(&inventory.error));
            before = allocation_count;
            assert(static_allocate(&inventory, 1) == NULL && allocation_count == before);
            assert(!static_invalid(&inventory, name, "later error", NULL));
            assert(inventory.status == STATIC_OUT_OF_MEMORY && inventory.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        }
        destroy_static();
    }
}

/** Each role consumes exactly one of the two groups. A full group refuses
 * before allocation, while insertion into the independent group still works.
 */
static void it_checks_both_interface_counter_limits_including_public_aliases(void) {
    for (StaticRole role = STATIC_PARAMETER; role <= STATIC_ALIAS; role++) {
        setup_static(STATIC_SCOPE_SOURCE_TEXT, STATIC_SCOPE_SOURCE_TEXT);
        StaticModule *module = static_module_add(&inventory, project.entry);
        assert(module != NULL);
        size_t *full = role == STATIC_PARAMETER ? &module->parameter_count : &module->result_count;
        size_t *other = role == STATIC_PARAMETER ? &module->result_count : &module->parameter_count;
        *full = SIZE_MAX;
        SourceSpan result = static_name_site(project.entry, "...result");
        StaticRole independent_role = role == STATIC_PARAMETER ? STATIC_RESULT : STATIC_PARAMETER;
        StaticSymbol *existing = static_symbol_add(&inventory, module, result, result,
            independent_role, STATIC_CATEGORY_TYPE);
        assert(existing != NULL && *full == SIZE_MAX && *other == 1);
        size_t before = allocation_count;
        SourceSpan name = static_name_site(project.entry, "...same");
        assert(static_symbol_add(&inventory, module, name, name, role, STATIC_CATEGORY_TYPE) == NULL);
        assert(inventory.status == STATIC_OUT_OF_MEMORY && allocation_count == before);
        assert(module->first == existing && module->last == existing && existing->next == NULL);
        assert(*full == SIZE_MAX && *other == 1);
        destroy_static();
    }
}

/** Execute all independent primitive contracts; no complete collector is claimed. */
void run(void) {
    it_keeps_exact_module_scoped_static_identities_and_global_alias_lookup();
    it_rejects_every_duplicate_role_pair_without_mutation_or_diagnostic_allocation();
    it_unwinds_every_static_record_allocation_failure_and_retains_the_first_error();
    it_checks_both_interface_counter_limits_including_public_aliases();
}

/** Return success only after every assertion and counted owner cleanup. */
int main(void) { run(); return EXIT_SUCCESS; }
