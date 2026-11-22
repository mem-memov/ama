/** @file
 * @brief Verify rooted fallback operands from exact targets/members, outside suffixes and fixed-site refusals.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include <stdio.h>
#include "../../../../../../tools/project/source/operands/rooted.h"
/** A normalized member is authoritative even when its raw spelling cannot be split into a module prefix. */
static void it_prepares_rooted_copied_and_retained_targets_without_rewriting_outside_operand_bytes(void) {
    const char *written[] = {"./relative", "../retained", "//a", "~alias"};
    const char *expected[] = {"//a", "/vendor/pkg/v1.0.0/src/parentish", "//a", "//a/chosen value"};
    for (unsigned choice = 0; choice < 4; choice++) {
        prepare(); binding_map(false);
        char bytes[256], wanted[256]; const char *suffix = "[]& # kept\r\n";
        assert(snprintf(bytes, sizeof(bytes), "%s%s", written[choice], suffix) > 0);
        assert(snprintf(wanted, sizeof(wanted), "%s%s", expected[choice], suffix) > 0);
        const SourceText *text = NULL;
        assert(source_manager_add(&sources, "operand", (const unsigned char *)bytes, strlen(bytes), &text) == SOURCE_OK);
        SourceTextEdits edit; source_text_edits_init(&edit, sources.allocator, text);
        DeliverySourceDependency edge = {.target = choice == 1 ? "/vendor/pkg/v1.0.0/src/parentish" : "/vendor/pkg/v1.0.0/src/parent/child",
            .member = choice == 3 ? "chosen value" : NULL, .origin = {text, 0, strlen(written[choice])}};
        SourceOperandIssue issue = {0}; size_t before = allocation_count;
        assert(source_operand_rooted(&edit, &map, &edge, &issue) == SOURCE_OPERAND_OK);
        if (choice == 2) { assert(!edit.first && allocation_count == before); }
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edit, &sources, "prepared operand", &result) == SOURCE_EDITS_OK);
        assert(source_length(result) == strlen(wanted) && !memcmp(source_bytes(result), wanted, strlen(wanted)));
        assert(!memcmp(source_bytes(text), bytes, strlen(bytes)));
        assert(source_operand_rooted(&edit, &map, &edge, &issue) == SOURCE_OPERAND_INVALID && edit.valid);
        source_text_edits_destroy(&edit); source_clone_map_destroy(&map); finish();
    }
}
/** Implicit requiring constructs are never replaced with a target path. */
static void it_keeps_fixed_dependencies_without_allocation_and_refuses_changed_fixed_identities(void) {
    for (unsigned changed = 0; changed < 3; changed++) {
        prepare(); source_clone_map_init(&map, sources.allocator, &capture);
        const char *identity = "/vendor/pkg/v1.0.0/src/parent/child";
        if (changed) {
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&map, original(identity), "/moved/child", changed == 1 ? "//changed" : identity, &row) == SOURCE_CLONE_OK);
        }
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        const SourceText *text = original(identity)->text;
        SourceTextEdits edit; source_text_edits_init(&edit, sources.allocator, text);
        DeliverySourceDependency edge = {.target = identity, .implicit = true, .origin = {text, 0, source_length(text)}};
        SourceOperandIssue issue = {0}; size_t before = allocation_count;
        assert(source_operand_rooted(&edit, &map, &edge, &issue) == (changed == 1 ? SOURCE_OPERAND_FIXED : SOURCE_OPERAND_OK));
        assert(!edit.first && allocation_count == before && issue.expected);
        if (changed == 1) {
            const SourceText *result = text;
            assert(edit.status == SOURCE_EDITS_CONFLICT && source_text_edits_apply(&edit, &sources, "prepared", &result) == SOURCE_EDITS_CONFLICT);
            assert(result == text);
        }
        source_text_edits_destroy(&edit); source_clone_map_destroy(&map); finish();
    }
}
/** All temporary member-join and durable edit allocations retain failure ownership and poison application. */
static void it_retires_every_rooted_operand_allocation_failure_and_rejects_invalid_handoffs(void) {
    size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        prepare(); binding_map(false);
        const SourceText *text = original("/vendor/pkg/v1.0.0/src/parent/child")->text;
        SourceTextEdits edit; source_text_edits_init(&edit, sources.allocator, text);
        DeliverySourceDependency edge = {.target = "/vendor/pkg/v1.0.0/src/parent/child", .member = "selected", .origin = {text, 0, source_length(text)}};
        SourceOperandIssue issue = {0}; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceOperandStatus status = source_operand_rooted(&edit, &map, &edge, &issue);
        if (!failure) { assert(status == SOURCE_OPERAND_OK); count = allocation_count - before; assert(count); }
        else { assert(status == SOURCE_OPERAND_EDITS && issue.edits_status == SOURCE_EDITS_MEMORY && edit.status == SOURCE_EDITS_MEMORY); }
        failure_at = 0; source_text_edits_destroy(&edit); source_clone_map_destroy(&map); finish();
    }
    for (unsigned choice = 0; choice < 10; choice++) {
        prepare(); binding_map(false);
        const SourceText *text = original("/vendor/pkg/v1.0.0/src/parent/child")->text;
        SourceTextEdits edit; source_text_edits_init(&edit, sources.allocator, text);
        DeliverySourceDependency edge = {.target = "/vendor/pkg/v1.0.0/src/parent/child", .origin = {text, 0, source_length(text)}};
        if (choice == 0) { edge.target = "relative"; }
        if (choice == 1) { edge.origin.source = NULL; }
        if (choice == 2) { edge.origin.end = 0; }
        if (choice == 3) { edge.member = ""; }
        if (choice == 4) { map.ready = false; }
        if (choice == 5) { edit.status = SOURCE_EDITS_MEMORY; }
        if (choice == 9) {
            assert(source_text_edits_add(&edit, edge.origin, (const unsigned char *)"blocked", 7) == SOURCE_EDITS_OK);
        }
        SourceOperandIssue issue = {0};
        assert(source_operand_rooted(NULL, &map, &edge, &issue) == SOURCE_OPERAND_INVALID);
        assert(source_operand_rooted(&edit, choice == 7 ? NULL : &map, choice == 8 ? NULL : &edge, choice == 6 ? NULL : &issue)
            == (choice == 5 || choice == 9 ? SOURCE_OPERAND_EDITS : SOURCE_OPERAND_INVALID));
        assert(edit.status != SOURCE_EDITS_OK && (edit.first != NULL) == (choice == 9));
        source_text_edits_destroy(&edit); source_clone_map_destroy(&map); finish();
    }
}
