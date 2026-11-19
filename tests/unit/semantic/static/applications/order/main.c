/** @file
 * @brief Compare interface-introduction canonical maps and residuals with actual input snapshots.
 */
#include <stdint.h>
#include "../../common.c"
#include "semantic/static/terms/store.h"
#include "semantic/static/bindings/owner.h"

/** Source introduces distinct ordinary Type parameters through input and result
 * declarations; internal storage order cannot replace original formal identity.
 */
static void it_keeps_interface_introduction_ordinals_in_captures_residuals_and_snapshots(void) {
    setup_static("@<\nfirst\n...implicit first\nsecond\n...explicit second\nlast\n...implicit last\n"
        "@>\nexported\n...implicit result\n@!\n", "@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    const char *names[] = {"...implicit first", "...explicit second", "...implicit last", "...implicit result"};
    const StaticSymbol *formals[4];
    for (size_t index = 0; index < 4; index++) {
        formals[index] = static_symbol_find(module, static_name_site(project.entry, names[index]));
        assert(formals[index] && formals[index]->role == STATIC_PARAMETER && formals[index]->ordinal == index);
        assert(static_symbol_at(module, STATIC_PARAMETER, index) == formals[index]);
    }
    assert(module->parameter_count == 4 && static_symbol_at(module, STATIC_RESULT, 0) == NULL);
    assert(static_symbol_at(module, STATIC_PARAMETER, 4) == NULL);
    assert(static_symbol_at(module, STATIC_RESULT, 1) == NULL);
    assert(static_symbol_at(module, STATIC_ALIAS, 0) == NULL);
    assert(static_symbol_at(module, (StaticRole)99, 0) == NULL);
    assert(static_symbol_at(NULL, STATIC_PARAMETER, 0) == NULL);
    assert(static_symbol_at(module, STATIC_PARAMETER, SIZE_MAX) == NULL);
    StaticTermStore store;
    static_term_store_init(&store, &inventory, sources.allocator);
    const StaticTerm *plain, *constructor, *complete, *partial;
    assert(static_term_nominal(&store, static_module_find(&inventory, project_find(&project, ".other")),
        NULL, 0, (SourceSpan){0}, &plain));
    assert(static_term_nominal(&store, module, NULL, 0, formals[0]->site, &constructor));
    const StaticTermResidual *residual = constructor->as.nominal.first_residual;
    const StaticSignatureEntry *entry = constructor->signature->first;
    for (size_t index = 0; index < 4; index++, residual = residual->next, entry = entry->next) {
        assert(residual && residual->formal == formals[index] && entry && layout_text_is(entry->name, names[index]));
    }
    StaticTermCapture captures[4] = {{0}};
    for (size_t index = 0; index < 4; index++) {
        captures[index].formal = formals[3 - index];
        captures[index].value = plain;
    }
    assert(static_term_nominal(&store, module, captures, 4, formals[0]->site, &complete));
    const StaticTermCapture *capture = complete->as.nominal.first_capture;
    for (size_t index = 0; index < 4; index++, capture = capture->next) {
        assert(capture && capture->formal == formals[index]);
    }
    StaticTermCapture fixed = {.formal = formals[2], .value = plain};
    assert(static_term_nominal(&store, module, &fixed, 1, formals[0]->site, &partial));
    BindingPlan plan;
    binding_plan_init(&plan, sources.allocator);
    BindingInterface *view;
    const StaticSymbol *fixed_keys[] = {formals[2]};
    assert(binding_source_interface(&plan, module, BINDING_MODULE_SURFACE, fixed_keys, 1, &view));
    const size_t expected[] = {0, 1, 3};
    residual = partial->as.nominal.first_residual;
    entry = partial->signature->first;
    const BindingSlot *slot = view->sequences[BINDING_STATIC_INPUT].first;
    for (size_t index = 0; index < 3; index++, residual = residual->next, entry = entry->next, slot = slot->next) {
        assert(residual && entry && slot && residual->formal == formals[expected[index]]);
        assert(slot->static_symbol == residual->formal && slot->sequence_ordinal == index);
        assert(layout_text_is(entry->name, names[expected[index]]));
    }
    assert(residual == NULL && entry == NULL && slot == NULL);
    binding_plan_destroy(&plan);
    static_term_store_destroy(&store);
    destroy_static();
}

/** Run the independently specified text-order/capture/snapshot agreement law. */
static void run(void) { it_keeps_interface_introduction_ordinals_in_captures_residuals_and_snapshots(); }
/** Return success after all assertions and complete counted teardown. */
int main(void) { run(); return 0; }
