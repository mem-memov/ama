/** @file
 * @brief Verify independent comparison scratch never mutates accepted source metadata.
 */
#include "../../common.c"
#include "semantic/static/signature/signature.h"

/** Reordered recursive foreign signatures compare exactly while their accepted
 * inventory keeps its original arena, status, definitions and metadata.
 */
static void it_compares_recursive_signatures_in_independent_read_only_scratch(void) {
    setup_static("@<\n...left\n ...key\n ...family\n  ...value\n  ...other\n"
        "...right\n ...family\n  ...other\n  ...value\n ...key\n@!\n", STATIC_SCOPE_SOURCE_TEXT);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    assert(module->first->site.start == static_name_site(project.entry, "...left").start);
    const StaticSignature *left = module->first->signature, *right = module->first->next->signature;
    SourceAllocation *original = inventory.arena.first;
    SourceArena scratch;
    source_arena_init(&scratch, sources.allocator);
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare_arena(&scratch, left, right, STATIC_SIGNATURE_EXACT, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && inventory.arena.first == original);
    assert(inventory.valid && inventory.status == STATIC_OK && module->first->signature == left);
    source_arena_destroy(&scratch);
    destroy_static();
}

/** Refuse each observed nested-comparison allocation; output and accepted
 * inventory remain unchanged and partially allocated scratch is destructible.
 */
static void it_preserves_output_and_source_inventory_on_each_signature_scratch_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static("@<\n...left\n ...family\n  ...value\n...right\n ...family\n  ...value\n@!\n", STATIC_SCOPE_SOURCE_TEXT);
        assert(static_inventory_collect(&inventory) == STATIC_OK);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        SourceAllocation *original = inventory.arena.first;
        SourceArena scratch;
        source_arena_init(&scratch, sources.allocator);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
        bool success = static_signature_compare_arena(&scratch, module->first->signature,
            module->first->next->signature, STATIC_SIGNATURE_EXACT, &result);
        if (!failure) { assert(success && result == STATIC_SIGNATURE_EQUAL); total = allocation_count - before; assert(total); }
        else { assert(!success && result == STATIC_SIGNATURE_DIFFERENT); }
        assert(inventory.arena.first == original && inventory.valid && inventory.status == STATIC_OK);
        failure_at = 0;
        source_arena_destroy(&scratch);
        destroy_static();
    }
}

/** Execute independent ownership and resource partitions. */
static void run(void) {
    it_compares_recursive_signatures_in_independent_read_only_scratch();
    it_preserves_output_and_source_inventory_on_each_signature_scratch_refusal();
}
/** Return success only after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
