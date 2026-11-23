/** @file
 * @brief Refuse missing, extra, duplicate and foreign destination permission sets without allocation.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Complete-set equality rejects both missing and extra permissions even when
 * ordinary copied-edge checking would not exercise an unused selected root. */
static void it_checks_complete_final_permission_sets_and_exact_scope_owners_without_allocation(void) {
    PermissionAdditionFixture fixture;
    prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, true, 0, false, false);
    SourceConfigurationCheckIssue issue;
    const SourcePackageScope *scope = fixture.scope;
    const DependencyViewEntry *chosen = scope->selection->roots->entry, *missing = NULL;
    assert(scope->selection->roots && !scope->selection->roots->next);
    for (size_t i = 0; i < fixture.final.view->count; i++) {
        if (&fixture.final.view->entries[i] != chosen) { missing = &fixture.final.view->entries[i]; break; }
    }
    assert(missing);
    const DependencyViewEntry *entries[] = {chosen, missing};
    SourceSelectionRoots requested = {entries, 1};
    size_t before = allocation_count; failure_at = before + 1;
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_OK);
    requested.count = 2;
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_MISSING && issue.expected == missing && !issue.actual);
    requested.count = 0;
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_EXTRA && issue.actual == chosen && !issue.expected);
    requested.count = 2; entries[1] = chosen;
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    DependencyViewEntry foreign = *chosen; entries[0] = &foreign; requested.count = 1;
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    entries[0] = chosen;
    SourcePackageScope foreign_scope = *scope;
    assert(source_copy_configuration_check(&fixture.final, &foreign_scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, &foreign_scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    const SourcePackageScope *other = environment_scope(&fixture.final, "/vendor/pkg/v1.0.0");
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, other, &issue) == SOURCE_CONFIGURATION_CHECK_SCOPE);
    assert(source_copy_configuration_check(NULL, scope, &requested, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    assert(source_copy_configuration_check(&fixture.final, scope, NULL, &fixture.final, scope, &issue) == SOURCE_CONFIGURATION_CHECK_INVALID);
    assert(source_copy_configuration_check(&fixture.final, scope, &requested, &fixture.final, scope, NULL) == SOURCE_CONFIGURATION_CHECK_INVALID);
    assert(allocation_count == before); failure_at = 0;
    finish_permission_additions(&fixture);
}
