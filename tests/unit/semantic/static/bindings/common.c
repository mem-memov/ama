/** @file
 * @brief Own binding metadata after accepted static source preparation and before source teardown.
 */
#ifndef AMA_BINDING_TEST_COMMON
#define AMA_BINDING_TEST_COMMON
#include "../common.c"
#include "semantic/static/bindings/internal.h"

/** Metadata ownership is independent from the immutable static inventory. */
static BindingPlan binding_plan;

/** Consume the source fixture's lexical stage before creating an independent,
 * allocation-free binding owner. No root layout is inferred here.
 */
static void complete_binding_setup(void) {
    assert(static_inventory_collect(&inventory) == STATIC_OK && inventory.valid);
    size_t before = allocation_count;
    binding_plan_init(&binding_plan, sources.allocator);
    assert(binding_plan.status == BINDING_OK && allocation_count == before);
}

/** Prepare the exact two-module domain and its binding owner. */
static void setup_binding(const char *main_text, const char *other_text) {
    setup_static(main_text, other_text);
    complete_binding_setup();
}

/** Admit explicit standard/import root families for genuine required sources;
 * optional fixture consumers introduce no unused function warnings.
 */
static inline void setup_binding_roots(const char *main_text, const char *other_text,
    const ModuleRoot *roots, size_t root_count) {
    setup_static_with_roots(main_text, other_text, roots, root_count);
    complete_binding_setup();
}

/** Obtain exact accepted module identities without allocating or matching layout. */
static const StaticModule *binding_module(const ProjectModule *definition) {
    const StaticModule *module = static_module_find(&inventory, definition);
    assert(module != NULL);
    return module;
}

/** Destroy snapshots before borrowed static/source records; counted project
 * teardown proves ownership also after unpublished interface failures. */
static void destroy_binding(void) {
    binding_plan_destroy(&binding_plan);
    assert(binding_plan.first_interface == NULL && binding_plan.last_interface == NULL);
    assert(inventory.valid && inventory.status == STATIC_OK);
    destroy_static();
}
#endif
