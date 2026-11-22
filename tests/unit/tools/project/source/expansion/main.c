/** @file
 * @brief Verify original structural facts through real catalog, compiler and closure consumers.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
/** Real source-cycle closure admits independent experiments and stable selected-version provenance. */
static void it_closes_actual_source_cycles_with_independent_tests_and_original_version_metadata(void) {
    prepare();
    AcquisitionClosureProvider provider = {.context = &expansion, .expand = source_expansion_prepare,
        .admit_test = source_expansion_admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &main_identity, 1, &provider);
    if (status != ACQUISITION_CLOSURE_OK) { fprintf(stderr, "Closure status %d: %s\n", status, closure.issue.message); }
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid);
    size_t tests = 0;
    for (const AcquisitionTestRoot *test = closure.tests; test; test = test->next) { tests++; }
    assert(tests == 2);
    bool configuration = false, guidance = false;
    const AcquisitionVersionSource *version = NULL;
    for (const AcquisitionMember *member = closure.members.first; member; member = member->next) {
        if (strstr(member->path, "/package.ini")) { configuration = true; }
        if (strstr(member->path, "/README.md")) { guidance = true; }
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            assert(cause->source && !cause->source->snapshot);
            if (!version) { version = cause->source; }
            assert(version == cause->source);
        }
    }
    assert(configuration && guidance); finish();
}
/** Draft ancestors remain context; selecting the draft itself retains opaque descendants. */
static void it_retains_draft_context_and_structural_subtrees_without_inventing_draft_tests(void) {
    prepare(); SourceIssue issue = {0}; AcquisitionExpansion fact = {0};
    assert(source_expansion_prepare(&expansion, child_identity, &fact, &issue) == ACQUISITION_CLOSURE_OK);
    assert(!fact.draft && fact.contexts && !fact.contexts->next && fact.test_files);
    assert(!strcmp(fact.contexts->identity, "/vendor/pkg/v1.0.0/src/parent"));
    assert(source_expansion_prepare(&expansion, "/vendor/pkg/v1.0.0/src/parent", &fact, &issue) == ACQUISITION_CLOSURE_OK);
    assert(fact.draft && !fact.test_directory && fact.submodule_count == 1);
    bool opaque = false;
    for (const AcquisitionUnitEntry *entry = fact.files; entry; entry = entry->next) {
        if (strstr(entry->file.path, "/opaque.data")) { opaque = true; }
    }
    assert(opaque); finish();
}
/** Exact input-owner joins and source membership refuse without output publication. */
static void it_refuses_unrelated_domains_and_unknown_identities_before_structural_capture(void) {
    prepare(); SourceIssue issue = {0}; AcquisitionExpansion fact = {.draft = true};
    const ModuleSnapshotEntry *saved = analysis.view.entries; analysis.view.entries = NULL;
    size_t before = allocation_count;
    assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    analysis.view.entries = saved;
    assert(source_expansion_prepare(NULL, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, NULL, &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, main_identity, NULL, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, main_identity, &fact, NULL) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, "./main", &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, "/vendor/pkg/v9.0.0", &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(source_expansion_prepare(&expansion, "/vendor/pkg/v1.0.0/src/missing", &fact, &issue) == ACQUISITION_CLOSURE_INVALID);
    assert(fact.draft && !fact.module && allocation_count == before); finish();
    assert(source_expansion_admit(NULL, NULL, &issue) == ACQUISITION_CLOSURE_INVALID && issue.message);
    assert(source_expansion_admit(NULL, NULL, NULL) == ACQUISITION_CLOSURE_INVALID);
}
/** Changed indexed bytes/companions/configuration cannot become new original facts. */
static void it_refuses_changed_original_unit_and_version_observations(void) {
    const char *paths[] = {"generated/project/packages/vendor/pkg/v1.0.0/src/main",
        "generated/project/packages/vendor/pkg/v1.0.0/src/main.extra",
        "generated/project/packages/vendor/pkg/v1.0.0/package.ini"};
    for (size_t index = 0; index < 3; index++) {
        prepare(); file(paths[index], "changed\n");
        SourceIssue issue = {0}; AcquisitionExpansion fact = {.draft = true};
        assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_CONFLICT);
        assert(fact.draft && !fact.module && issue.message && expansion.failed_path);
        finish();
        if (index == 1) { assert(unlink(paths[index]) == 0); }
    }
}
/** Test helpers and standard units retain their own bytes without tests-of-tests or package guesses. */
static void it_prepares_standard_and_test_units_without_ordinary_source_associations(void) {
    prepare(); SourceIssue issue = {0}; AcquisitionExpansion fact = {0};
    assert(source_expansion_prepare(&expansion, "//plain", &fact, &issue) == ACQUISITION_CLOSURE_OK);
    assert(!fact.source && !fact.configuration && !fact.guidance && !fact.test_directory && !fact.test_files && !fact.draft);
    assert(source_expansion_prepare(&expansion, "/vendor/pkg/v1.0.0/tests/main/check", &fact, &issue) == ACQUISITION_CLOSURE_OK);
    assert(fact.source && fact.configuration && !fact.test_directory && !fact.test_files && !fact.draft);
    finish();
}
/** Optional guidance may be absent; supplied directories and missing experiment trees remain failures. */
static void it_distinguishes_absent_guidance_from_invalid_guidance_and_missing_test_material(void) {
    const char *readme = "generated/project/packages/vendor/pkg/v1.0.0/README.md";
    prepare(); assert(unlink(readme) == 0);
    SourceIssue issue = {0}; AcquisitionExpansion fact = {0};
    assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_OK);
    assert(!fact.guidance && fact.configuration); finish();
    prepare(); assert(unlink(readme) == 0); directory(readme);
    fact = (AcquisitionExpansion){.draft = true};
    assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED);
    assert(fact.draft && !fact.module && expansion.failed_path); finish(); assert(rmdir(readme) == 0);
    prepare();
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/main") == 0);
    assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_PREPARATION_FAILED);
    assert(fact.draft && !fact.module); finish();
}
/** Byte equality cannot admit a replacement indexed inode or a disappeared companion. */
static void it_rejects_replaced_main_identity_and_removed_original_companions(void) {
    prepare();
    const char *main = "generated/project/packages/vendor/pkg/v1.0.0/src/main";
    assert(rename(main, "generated/prior-main") == 0);
    file(main, "@!\nother&\n./other\n");
    SourceIssue issue = {0}; AcquisitionExpansion fact = {.draft = true};
    assert(source_expansion_prepare(&expansion, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_CONFLICT);
    assert(fact.draft && !fact.module); finish(); assert(unlink("generated/prior-main") == 0);
    prepare(); assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parent.extra") == 0);
    assert(source_expansion_prepare(&expansion, "/vendor/pkg/v1.0.0/src/parent", &fact, &issue) == ACQUISITION_CLOSURE_CONFLICT);
    assert(fact.draft && !fact.module); finish();
}
/** Whole-version preparation retains opaque trees and actual source experiments; empty versions stay empty. */
static void it_closes_whole_versions_and_empty_selected_source_roots(void) {
    const char *versions[] = {"/vendor/pkg/v1.0.0", "/vendor/pkg/v2.0.0"};
    for (size_t index = 0; index < 2; index++) {
        prepare();
        AcquisitionClosureProvider provider = {.context = &expansion, .expand = source_expansion_prepare,
            .admit_test = source_expansion_admit};
        assert(acquisition_closure_compute(&closure, &versions[index], 1, &provider) == ACQUISITION_CLOSURE_OK);
        size_t tests = 0; bool opaque = false, directory_root = false;
        for (const AcquisitionTestRoot *test = closure.tests; test; test = test->next) { tests++; }
        for (const AcquisitionMember *entry = closure.members.first; entry; entry = entry->next) {
            if (strstr(entry->path, "/opaque.data")) { opaque = true; }
            size_t length = strlen(entry->path);
            if (entry->directory && length >= 4 && !strcmp(entry->path + length - 4, "/src")) { directory_root = true; }
        }
        assert(directory_root && tests == (index ? 0 : 3) && opaque == !index);
        finish();
    }
}
/** Every measured structural allocation failure retains destructible scratch and unchanged output. */
static void it_retires_all_structural_preparation_allocation_failures(void) {
    const char *identities[] = {child_identity, "/vendor/pkg/v1.0.0", "/vendor/pkg/v2.0.0"};
    for (size_t selection = 0; selection < 3; selection++) {
        prepare(); SourceIssue issue = {0}; AcquisitionExpansion fact = {0}; size_t before = allocation_count;
        assert(source_expansion_prepare(&expansion, identities[selection], &fact, &issue) == ACQUISITION_CLOSURE_OK);
        size_t count = allocation_count - before; assert(count); finish();
        for (size_t index = 1; index <= count; index++) {
            prepare(); failure_at = allocation_count + index; fact = (AcquisitionExpansion){.draft = true};
            assert(source_expansion_prepare(&expansion, identities[selection], &fact, &issue) != ACQUISITION_CLOSURE_OK);
            assert(fact.draft && !fact.module); failure_at = 0; finish();
        }
    }
}
#include "prepared-graph.c"
#include "declared-inputs.c"
/** Run real provider/closure joins, draft topology, original freshness and failure ownership. */
void run(void) {
    it_closes_actual_source_cycles_with_independent_tests_and_original_version_metadata();
    it_retains_draft_context_and_structural_subtrees_without_inventing_draft_tests();
    it_refuses_unrelated_domains_and_unknown_identities_before_structural_capture();
    it_refuses_changed_original_unit_and_version_observations();
    it_prepares_standard_and_test_units_without_ordinary_source_associations();
    it_distinguishes_absent_guidance_from_invalid_guidance_and_missing_test_material();
    it_rejects_replaced_main_identity_and_removed_original_companions();
    it_closes_whole_versions_and_empty_selected_source_roots();
    it_retires_all_structural_preparation_allocation_failures();
    it_reparses_prepared_cyclic_copies_and_checks_real_binding_and_site_provenance();
    it_closes_actual_declared_module_cycles_with_exact_original_handles_and_no_semantic_clones();
    it_retains_explicit_foreign_version_and_unversioned_opaque_input_origins();
    it_refuses_invalid_or_changed_global_declared_groups_without_capture_fallback();
    it_retires_every_declared_expansion_allocation_in_module_and_foreign_asset_compositions();
}
/** The leaf runner owns fixture directories and cleans successful generated output. */
int main(void) { run(); return 0; }
