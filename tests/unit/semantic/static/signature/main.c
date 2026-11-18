/** @file
 * @brief Verify explicit finite signatures against independent structural partitions.
 */
#include "../common.c"
#include "semantic/static/signature/signature.h"

/** Two identical foreign name sets deliberately reverse every sibling order. */
static const char *signature_source =
    "@<\n...left\n ...key\n ...family\n  ...value\n  ...other\n"
    "...right\n ...family\n  ...other\n  ...value\n ...key\n@!\n";

/** Build two parsed constructor signatures without introducing their members
 * into the local table. Output records own metadata, never source strings.
 */
static void signatures(StaticSignature **left, StaticSignature **right) {
    const AstNode *node = project.entry->syntax.parameters.first;
    assert(node != NULL && node->next != NULL);
    assert(static_signature_build(&inventory, node->as.static_formal->first, left));
    assert(static_signature_build(&inventory, node->next->as.static_formal->first, right));
}

/** Exact recursive sets ignore source ordering and preserve foreign provenance. */
static void it_builds_finite_foreign_signatures_and_matches_reordered_name_sets(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticSignature *left = NULL, *right = NULL;
    signatures(&left, &right);
    assert(left != right && left->count == 2 && right->count == 2);
    assert(layout_text_is(left->first->name, "...key"));
    assert(left->first->category == STATIC_CATEGORY_TYPE && left->first->signature == NULL);
    assert(left->last->category == STATIC_CATEGORY_TEMPLATE && left->last->signature->count == 2);
    assert(left->last->pending_expression == NULL && inventory.first == NULL);
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &result));
    assert(result == STATIC_SIGNATURE_EQUAL && !inventory.valid);
    assert(static_signature_compare(&inventory, right, left, STATIC_SIGNATURE_EXACT, &result));
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
            case 1: right->last->name = static_name_site(project.entry, "...left"); break;
            case 2: right->first->signature->last->name = static_name_site(project.entry, "...left"); break;
            case 3: right->first->category = STATIC_CATEGORY_TYPE; break;
            case 4: right->last->category = STATIC_CATEGORY_DEFERRED; break;
            case 5: left->first->category = STATIC_CATEGORY_DEFERRED; break;
            case 6:
                left->first->category = STATIC_CATEGORY_DEFERRED;
                right->first->signature->last->name = static_name_site(project.entry, "...left");
                break;
        }
        StaticSignatureComparison result = STATIC_SIGNATURE_EQUAL;
        assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &result));
        assert(result == ((partition == 4 || partition == 5) ? STATIC_SIGNATURE_PENDING : STATIC_SIGNATURE_DIFFERENT));
        assert(static_signature_compare(&inventory, right, left, STATIC_SIGNATURE_EXACT, &result));
        assert(result == ((partition == 4 || partition == 5) ? STATIC_SIGNATURE_PENDING : STATIC_SIGNATURE_DIFFERENT));
        assert(inventory.status == STATIC_OK);
        destroy_static();
    }
}

/** Duplicate foreign slots fail within either root or nested signature, while
 * identical spelling at distinct levels remains valid and does not capture.
 */
static void it_rejects_same_level_signature_duplicates_at_original_sites(void) {
    const char *cases[] = {
        "@<\n...family\n ...slot\n ...slot\n@!\n",
        "@<\n...family\n ...nested\n  ...slot\n  ...slot\n@!\n"
    };
    for (size_t index = 0; index < 2; index++) {
        setup_static(cases[index], STATIC_SCOPE_SOURCE_TEXT);
        StaticSignature *output = NULL;
        assert(!static_signature_build(&inventory,
            project.entry->syntax.parameters.first->as.static_formal->first, &output));
        assert(output == NULL && inventory.status == STATIC_INVALID);
        assert(diagnostic_valid(&inventory.error) && inventory.error.related_count == 1);
        assert(layout_text_is(inventory.error.primary.span, "...slot"));
        assert(layout_text_is(inventory.related.location.span, "...slot"));
        assert(inventory.error.primary.span.start > inventory.related.location.span.start);
        destroy_static();
    }
    setup_static("@<\n...family\n ...slot\n  ...slot\n@!\n", STATIC_SCOPE_SOURCE_TEXT);
    StaticSignature *output = NULL;
    assert(static_signature_build(&inventory,
        project.entry->syntax.parameters.first->as.static_formal->first, &output));
    assert(output->count == 1 && output->first->signature->count == 1);
    destroy_static();
}

/** Every observed allocation refusal leaves caller outputs untouched; no
 * partially built tree or interrupted comparison certifies a usable inventory.
 */
static void it_unwinds_every_signature_build_and_compare_allocation_failure(void) {
    for (size_t operation = 0; operation < 2; operation++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
            StaticSignature *left = NULL, *right = NULL, *output = NULL;
            if (operation == 1) { signatures(&left, &right); }
            StaticSignatureComparison comparison = STATIC_SIGNATURE_DIFFERENT;
            size_t before = allocation_count;
            if (failure != 0) { failure_at = before + failure; }
            bool success = operation == 0 ? static_signature_build(&inventory,
                project.entry->syntax.parameters.first->as.static_formal->first, &output) :
                static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &comparison);
            if (failure == 0) { assert(success); total = allocation_count - before; assert(total != 0); }
            else {
                assert(!success && inventory.status == STATIC_OUT_OF_MEMORY && !inventory.valid);
                assert(output == NULL && comparison == STATIC_SIGNATURE_DIFFERENT);
                assert(diagnostic_valid(&inventory.error));
            }
            destroy_static();
        }
    }
}

/** Deep syntax exercises explicit work frames in construction and comparison;
 * this fixture depth is evidence, never a language nesting limit.
 */
static void it_builds_and_compares_deep_signatures_without_recursive_c_activation(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    enum { DEPTH = 900 };
    AstStaticFormal formals[DEPTH];
    memset(formals, 0, sizeof(formals));
    SourceSpan name = static_name_site(project.entry, "...key");
    for (size_t index = 0; index < DEPTH; index++) {
        formals[index].name = name;
        if (index + 1 < DEPTH) { formals[index].first = &formals[index + 1]; }
    }
    StaticSignature *left = NULL, *right = NULL;
    assert(static_signature_build(&inventory, formals, &left));
    assert(static_signature_build(&inventory, formals, &right));
    StaticSignatureComparison result = STATIC_SIGNATURE_DIFFERENT;
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &result));
    assert(result == STATIC_SIGNATURE_EQUAL);
    StaticSignature *cursor = left;
    for (size_t index = 0; index < DEPTH; index++) {
        assert(cursor != NULL && cursor->count == 1);
        cursor = cursor->first->signature;
    }
    assert(cursor == NULL);
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
    SourceSpan names[] = {static_name_site(project.entry, "...key"), static_name_site(project.entry, "...family")};
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
        assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &result));
        assert(result == (pending == 0 ? STATIC_SIGNATURE_EQUAL : STATIC_SIGNATURE_PENDING));
        assert(allocation_count - before == LEVELS);
    }
    destroy_static();
}

/** Provided mode permits only a root subset; nested constructor signatures
 * retain exact finite name sets. Root excess and renamed names still fail.
 */
static void it_limits_partial_signature_matching_to_supplied_root_names(void) {
    setup_static(signature_source, STATIC_SCOPE_SOURCE_TEXT);
    StaticSignature *left = NULL, *right = NULL;
    signatures(&left, &right);
    left->count = 1;
    left->first = left->last;
    StaticSignatureComparison result;
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_PROVIDED, &result));
    assert(result == STATIC_SIGNATURE_EQUAL);
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_EXACT, &result));
    assert(result == STATIC_SIGNATURE_DIFFERENT);
    assert(static_signature_compare(&inventory, right, left, STATIC_SIGNATURE_PROVIDED, &result));
    assert(result == STATIC_SIGNATURE_DIFFERENT);
    left->first->signature->count--;
    left->first->signature->first = left->first->signature->last;
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_PROVIDED, &result));
    assert(result == STATIC_SIGNATURE_DIFFERENT);
    left->first->name = static_name_site(project.entry, "...left");
    assert(static_signature_compare(&inventory, left, right, STATIC_SIGNATURE_PROVIDED, &result));
    assert(result == STATIC_SIGNATURE_DIFFERENT);
    destroy_static();
}

/** Execute every independent signature partition explicitly. */
static void run(void) {
    it_builds_finite_foreign_signatures_and_matches_reordered_name_sets();
    it_distinguishes_missing_renamed_nested_category_and_pending_slots();
    it_rejects_same_level_signature_duplicates_at_original_sites();
    it_unwinds_every_signature_build_and_compare_allocation_failure();
    it_builds_and_compares_deep_signatures_without_recursive_c_activation();
    it_memoizes_shared_signature_pairs_without_skipping_pending_metadata();
    it_limits_partial_signature_matching_to_supplied_root_names();
}

/** Return success only after all assertions and counted teardown pass. */
int main(void) { run(); return 0; }
