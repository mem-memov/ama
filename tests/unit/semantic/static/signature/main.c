/** @file
 * @brief Verify actual finite metadata signatures against independent structural partitions.
 */
#include "../common.c"
#include "semantic/static/signature/signature.h"

/** Current ordinary Type parameters supply borrowed names for independent metadata
 * fixtures. No source declaration of an expected constructor signature exists.
 */
static const char *signature_source =
    "@<\nleft\n.left\nright\n.right\nkey\n.key\nfamily\n.family\n"
    "value\n.value\nother\n.other\n@!\n";

/** Construct independent finite name sets in opposite sibling orders. These
 * literal metadata fixtures exercise comparison, not a removed AST builder.
 */
static void signatures(StaticSignature **left, StaticSignature **right) {
    StaticSignature *roots = static_allocate(&inventory, 4 * sizeof(*roots));
    StaticSignatureEntry *slots = static_allocate(&inventory, 8 * sizeof(*slots));
    assert(roots != NULL && slots != NULL);
    const char *names[] = {".key", ".family", ".value", ".other",
        ".family", ".key", ".other", ".value"};
    for (size_t index = 0; index < 8; index++) {
        slots[index].name = static_name_site(project.entry, names[index]);
        slots[index].category = STATIC_CATEGORY_TYPE;
        if (index % 2 == 0) { slots[index].next = &slots[index + 1]; }
    }
    for (size_t index = 0; index < 4; index++) {
        roots[index] = (StaticSignature){&slots[2 * index], &slots[2 * index + 1], 2};
    }
    slots[1].category = slots[4].category = STATIC_CATEGORY_TEMPLATE;
    slots[1].signature = &roots[1];
    slots[4].signature = &roots[3];
    *left = &roots[0];
    *right = &roots[2];
}

/** Exact recursive sets ignore source ordering and preserve foreign provenance. */
static void it_matches_reordered_finite_actual_signature_name_sets(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticSignature *left = NULL, *right = NULL;
    signatures(&left, &right);
    assert(left != right && left->count == 2 && right->count == 2);
    assert(layout_text_is(left->first->name, ".key"));
    assert(left->first->category == STATIC_CATEGORY_TYPE && left->first->signature == NULL);
    assert(left->last->category == STATIC_CATEGORY_TEMPLATE && left->last->signature->count == 2);
    assert(inventory.first == NULL);
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare_arena(&inventory.arena, left, right, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && !inventory.valid);
    assert(static_signature_compare_arena(&inventory.arena, right, left, &result));
    assert(result == STATIC_SIGNATURE_EQUAL);
    destroy_static();
}

/** Each partition changes one independent mathematical equality condition.
 * Deferred metadata is symmetric and does not hide a separate known mismatch.
 */
static void it_distinguishes_missing_renamed_nested_category_and_pending_slots(void) {
    for (size_t partition = 0; partition < 7; partition++) {
        setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
        StaticSignature *left = NULL, *right = NULL;
        signatures(&left, &right);
        switch (partition) {
            case 0: right->count--; right->first = right->last; break;
            case 1: right->last->name = static_name_site(project.entry, ".left"); break;
            case 2: right->first->signature->last->name = static_name_site(project.entry, ".left"); break;
            case 3: right->first->category = STATIC_CATEGORY_TYPE; break;
            case 4: right->last->category = STATIC_CATEGORY_DEFERRED; break;
            case 5: left->first->category = STATIC_CATEGORY_DEFERRED; break;
            case 6:
                left->first->category = STATIC_CATEGORY_DEFERRED;
                right->first->signature->last->name = static_name_site(project.entry, ".left");
                break;
        }
        StaticSignatureComparison result = STATIC_SIGNATURE_EQUAL;
        assert(static_signature_compare_arena(&inventory.arena, left, right, &result));
        assert(result == ((partition == 4 || partition == 5) ? STATIC_SIGNATURE_PENDING : STATIC_SIGNATURE_DIFFERENT));
        assert(static_signature_compare_arena(&inventory.arena, right, left, &result));
        assert(result == ((partition == 4 || partition == 5) ? STATIC_SIGNATURE_PENDING : STATIC_SIGNATURE_DIFFERENT));
        assert(inventory.status == STATIC_OK);
        destroy_static();
    }
}

/** Every comparison allocation refusal preserves the caller result and leaves
 * owned scratch available to counted teardown; fixtures are prepared first.
 */
static void it_unwinds_every_signature_comparison_allocation_failure(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
        StaticSignature *left = NULL, *right = NULL;
        signatures(&left, &right);
        StaticSignatureComparison comparison = STATIC_SIGNATURE_DIFFERENT;
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = static_signature_compare_arena(&inventory.arena, left, right,
            &comparison);
        if (failure == 0) { assert(success); total = allocation_count - before; assert(total != 0); }
        else {
            assert(!success && inventory.status == STATIC_OK && !inventory.valid);
            assert(comparison == STATIC_SIGNATURE_DIFFERENT);
        }
        destroy_static();
    }
}

/** Deep finite actual metadata checks iterative comparison independently of
 * source syntax. This depth is evidence, never a semantic nesting limit.
 */
static void it_compares_deep_signatures_without_recursive_c_activation(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    enum { DEPTH = 900 };
    StaticSignature roots[DEPTH][2];
    StaticSignatureEntry slots[DEPTH][2];
    memset(roots, 0, sizeof(roots));
    memset(slots, 0, sizeof(slots));
    SourceSpan name = static_name_site(project.entry, ".key");
    for (size_t index = 0; index < DEPTH; index++) {
        for (size_t side = 0; side < 2; side++) {
            roots[index][side] = (StaticSignature){&slots[index][side], &slots[index][side], 1};
            slots[index][side].name = name;
            slots[index][side].category = index + 1 < DEPTH ? STATIC_CATEGORY_TEMPLATE : STATIC_CATEGORY_TYPE;
            if (index + 1 < DEPTH) { slots[index][side].signature = &roots[index + 1][side]; }
        }
    }
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare_arena(&inventory.arena, &roots[0][0], &roots[0][1], &result));
    assert(result == STATIC_SIGNATURE_EQUAL);
    destroy_static();
}

/** Shared DAGs denote exponentially large unfolded trees. The comparison must
 * visit each exact pair once while still preserving deferred leaf obligations.
 */
static void it_memoizes_shared_signature_pairs_without_skipping_pending_metadata(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    enum { LEVELS = 100 };
    StaticSignature left[LEVELS], right[LEVELS];
    StaticSignatureEntry left_slots[LEVELS][2], right_slots[LEVELS][2];
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));
    memset(left_slots, 0, sizeof(left_slots));
    memset(right_slots, 0, sizeof(right_slots));
    SourceSpan names[] = {static_name_site(project.entry, ".key"), static_name_site(project.entry, ".family")};
    for (size_t level = 0; level < LEVELS; level++) {
        left[level] = (StaticSignature){left_slots[level], &left_slots[level][1], 2};
        right[level] = (StaticSignature){right_slots[level], &right_slots[level][1], 2};
        for (size_t slot = 0; slot < 2; slot++) {
            left_slots[level][slot] = (StaticSignatureEntry){.name = names[slot],
                .next = slot == 0 ? &left_slots[level][1] : NULL,
                .category = level + 1 < LEVELS ? STATIC_CATEGORY_TEMPLATE : STATIC_CATEGORY_TYPE,
                .signature = level + 1 < LEVELS ? &left[level + 1] : NULL};
            right_slots[level][slot] = left_slots[level][slot];
            right_slots[level][slot].next = slot == 0 ? &right_slots[level][1] : NULL;
            right_slots[level][slot].signature = level + 1 < LEVELS ? &right[level + 1] : NULL;
        }
    }
    for (size_t pending = 0; pending < 2; pending++) {
        if (pending != 0) { right_slots[LEVELS - 1][1].category = STATIC_CATEGORY_DEFERRED; }
        size_t before = allocation_count;
        StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
        assert(static_signature_compare_arena(&inventory.arena, left, right, &result));
        assert(result == (pending == 0 ? STATIC_SIGNATURE_EQUAL : STATIC_SIGNATURE_PENDING));
        assert(allocation_count - before == LEVELS);
    }
    destroy_static();
}

/** Known actual metadata admits shared DAGs and refuses active ancestor edges.
 * Each measured traversal allocation is refused independently, preserving owned
 * scratch and the exact distinction between resource failure and a cycle.
 */
static void it_checks_actual_signature_cycles_sharing_and_every_traversal_refusal(void) {
    for (size_t shape = 0; shape < 3; shape++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
            StaticModule *module = static_module_add(&inventory, project.entry);
            assert(module != NULL);
            StaticSignature roots[3] = {{0}};
            StaticSignatureEntry slots[4] = {{0}};
            SourceSpan name = static_name_site(project.entry, ".key");
            for (size_t index = 0; index < 4; index++) {
                slots[index].name = name;
                slots[index].category = STATIC_CATEGORY_TEMPLATE;
            }
            slots[1].name = static_name_site(project.entry, ".family");
            roots[0] = (StaticSignature){&slots[0], &slots[1], 2};
            roots[1] = (StaticSignature){&slots[2], &slots[2], 1};
            roots[2] = (StaticSignature){&slots[3], &slots[3], 1};
            slots[0].next = &slots[1];
            slots[0].signature = &roots[1];
            slots[1].signature = slots[2].signature = &roots[2];
            if (shape == 0) { slots[3].category = STATIC_CATEGORY_TYPE; }
            else { slots[3].signature = &roots[shape == 1 ? 2 : 0]; }
            module->constructor_signature = &roots[0];
            size_t before = allocation_count;
            if (failure != 0) { failure_at = before + failure; }
            bool success = static_signatures_acyclic(&inventory);
            if (failure == 0) {
                assert(success == (shape == 0));
                total = allocation_count - before;
                assert(total == 3);
                assert(inventory.status == (shape == 0 ? STATIC_OK : STATIC_INVALID));
                if (shape != 0) { assert(strstr(inventory.error.message, "Cyclic") != NULL); }
            } else {
                assert(!success && inventory.status == STATIC_OUT_OF_MEMORY);
                assert(diagnostic_valid(&inventory.error));
            }
            destroy_static();
        }
    }
}

/** Execute every independent signature partition explicitly. */
static void run(void) {
    it_matches_reordered_finite_actual_signature_name_sets();
    it_distinguishes_missing_renamed_nested_category_and_pending_slots();
    it_unwinds_every_signature_comparison_allocation_failure();
    it_compares_deep_signatures_without_recursive_c_activation();
    it_memoizes_shared_signature_pairs_without_skipping_pending_metadata();
    it_checks_actual_signature_cycles_sharing_and_every_traversal_refusal();
}

/** Return success only after all assertions and counted teardown pass. */
int main(void) { run(); return 0; }
