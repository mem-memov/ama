/** @file
 * @brief Compose explicit original input ownership with real structural expansion and declared-module closure.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Resolve exact original handles rather than copying source descriptors. */
static const SourceCapturedUnit *declared_unit(const char *identity) {
    for (const SourceCapturedUnit *unit = capture.units; unit; unit = unit->next) {
        if (!strcmp(unit->source->logical_path, identity)) { return unit; }
    }
    assert(false); return NULL;
}
/** The second selected version is deliberately empty but has real package metadata. */
static const ModuleVersion *declared_version(void) {
    for (const ModuleVersion *version = capture.catalog.versions; version; version = version->next) {
        if (!strcmp(version->logical_path, "/vendor/pkg/v2.0.0")) { return version; }
    }
    assert(false); return NULL;
}
/** Explicitly retain every semantic dependency externally; data requirements remain separate. */
static bool no_semantic_clones(void *context, const DeliverySourceDefinition *owner, const DeliverySourceDependency *edge) {
    (void)context; assert(owner && edge); return false;
}
/** Capture one opaque version-local input; its invalid UTF-8 must never be parsed as Ama. */
static const char *declared_asset(SourceArena *metadata, SourceManager *files, AcquisitionUnitEntry *entry) {
    const char *parent = declared_version()->physical_path;
    const char *path = source_arena_join(metadata, parent, "/", "declared.data"); assert(path);
    file(path, "\xff explicit opaque input\n"); *entry = (AcquisitionUnitEntry){0};
    assert(delivery_file_capture(metadata, files, parent, "declared.data", &entry->file) == SOURCE_OK);
    assert(entry->file.state == DELIVERY_FILE_PRESENT && entry->file.issue == PATH_FOUND);
    return path;
}
/** Explicit data-main cycles activate both units/tests once without semantic dependency cloning. */
static void it_closes_actual_declared_module_cycles_with_exact_original_handles_and_no_semantic_clones(void) {
    prepare(); const SourceCapturedUnit *main = declared_unit(main_identity);
    const SourceCapturedUnit *other = declared_unit("/vendor/pkg/v1.0.0/src/other");
    SourceDeclaredGroup back = {.requester = other, .version = main->source->version, .files = main->files};
    SourceDeclaredGroup forward = {.next = &back, .requester = main, .version = other->source->version, .files = other->files};
    SourceExpansion provider; source_expansion_init(&provider, sources.allocator, &capture, &analysis, &forward);
    AcquisitionClosure selected; acquisition_closure_init(&selected, sources.allocator);
    AcquisitionClosureProvider callbacks = {.context = &provider, .expand = source_expansion_prepare,
        .admit_test = source_expansion_admit, .follow_dependency = no_semantic_clones};
    assert(acquisition_closure_compute(&selected, &main_identity, 1, &callbacks) == ACQUISITION_CLOSURE_OK && selected.valid);
    size_t units = 0, tests = 0, declared_causes = 0;
    for (const AcquisitionRoleRecord *role = selected.roles.records; role; role = role->next) {
        assert(role->role == ACQUISITION_ACTIVE && role->expanded); units++;
        for (const AcquisitionReason *reason = role->reasons; reason; reason = reason->next) {
            assert(reason->kind != ACQUISITION_DEPENDENCY);
            if (reason->kind == ACQUISITION_DECLARED_MODULE) {
                assert(reason->origin.source && reason->origin.start == 0 && reason->origin.end == 0); declared_causes++;
            }
        }
    }
    for (const AcquisitionTestRoot *test = selected.tests; test; test = test->next) { tests++; }
    assert(units == 4 && tests == 2 && declared_causes == 2 && !acquisition_roles_take(&selected.roles));
    acquisition_closure_destroy(&selected); source_expansion_destroy(&provider); finish();
}
/** Input origins are explicit: neither a foreign version nor NULL borrows the requesting module's version. */
static void it_retains_explicit_foreign_version_and_unversioned_opaque_input_origins(void) {
    for (unsigned bound = 0; bound < 2; bound++) {
        prepare(); SourceArena metadata; source_arena_init(&metadata, sources.allocator);
        SourceManager files; source_manager_init(&files, sources.allocator);
        AcquisitionUnitEntry entry; const char *path = declared_asset(&metadata, &files, &entry);
        SourceDeclaredGroup group = {.requester = declared_unit(main_identity), .files = &entry,
            .version = bound ? declared_version() : NULL};
        SourceExpansion provider; source_expansion_init(&provider, sources.allocator, &capture, &analysis, &group);
        AcquisitionExpansion fact = {0}; SourceIssue issue = {0};
        assert(source_expansion_prepare(&provider, main_identity, &fact, &issue) == ACQUISITION_CLOSURE_OK);
        assert(fact.declared_inputs && !fact.declared_inputs->next && !fact.declared_modules);
        assert(fact.declared_inputs->files == &entry && fact.source && !strcmp(fact.source->version, "/vendor/pkg/v1.0.0"));
        if (bound) { assert(fact.declared_inputs->source && !strcmp(fact.declared_inputs->source->version, "/vendor/pkg/v2.0.0")); }
        else { assert(!fact.declared_inputs->source); }
        source_expansion_destroy(&provider); assert(unlink(path) == 0);
        source_manager_destroy(&files); source_arena_destroy(&metadata); finish();
    }
}
/** Foreign owners/origins and contradictory known bytes refuse before structural capture or output publication. */
static void it_refuses_invalid_or_changed_global_declared_groups_without_capture_fallback(void) {
    for (unsigned choice = 0; choice < 4; choice++) {
        prepare(); const SourceCapturedUnit *main = declared_unit(main_identity), *other = declared_unit("/vendor/pkg/v1.0.0/src/other");
        SourceCapturedUnit foreign = *main; ModuleVersion foreign_version = *main->source->version;
        AcquisitionUnitEntry entry = *other->files; entry.next = NULL;
        SourceDeclaredGroup group = {.requester = main, .version = main->source->version, .files = &entry};
        if (choice == 0) { group.requester = &foreign; }
        if (choice == 3) { group.version = &foreign_version; }
        if (choice == 2) { entry.file.path = source_path(main->source->version->configuration); }
        if (choice == 1 || choice == 2) {
            assert(source_manager_add(&sources, entry.file.path, (const unsigned char *)"changed", 7, &entry.file.content) == SOURCE_OK);
        }
        SourceExpansion provider; source_expansion_init(&provider, sources.allocator, &capture, &analysis, &group);
        AcquisitionExpansion fact = {.draft = true}; SourceIssue issue = {0}; size_t before = allocation_count;
        assert(source_expansion_prepare(&provider, main_identity, &fact, &issue)
            == (choice == 0 || choice == 3 ? ACQUISITION_CLOSURE_INVALID : ACQUISITION_CLOSURE_CONFLICT));
        assert(fact.draft && !fact.module && !provider.declarations_checked && allocation_count == before);
        if (choice == 1 || choice == 2) { assert(provider.failed_path == entry.file.path); }
        source_expansion_destroy(&provider); finish();
    }
}
/** Fault every structural/declaration/origin metadata allocation using immutable healthy input groups. */
static void it_retires_every_declared_expansion_allocation_in_module_and_foreign_asset_compositions(void) {
    for (unsigned asset = 0; asset < 2; asset++) {
        size_t count = 0;
        for (size_t failure = 0; !failure || failure <= count; failure++) {
            prepare(); SourceArena metadata; source_arena_init(&metadata, sources.allocator);
            SourceManager files; source_manager_init(&files, sources.allocator);
            AcquisitionUnitEntry entry; const char *path = NULL;
            const SourceCapturedUnit *other = declared_unit("/vendor/pkg/v1.0.0/src/other");
            if (asset) { path = declared_asset(&metadata, &files, &entry); }
            SourceDeclaredGroup group = {.requester = declared_unit(main_identity), .files = asset ? &entry : other->files,
                .version = asset ? declared_version() : other->source->version};
            SourceExpansion provider; source_expansion_init(&provider, sources.allocator, &capture, &analysis, &group);
            AcquisitionExpansion fact = {.draft = true}; SourceIssue issue = {0}; size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            AcquisitionClosureStatus status = source_expansion_prepare(&provider, main_identity, &fact, &issue);
            if (!failure) { assert(status == ACQUISITION_CLOSURE_OK); count = allocation_count - before; assert(count); }
            else { assert(status == ACQUISITION_CLOSURE_OUT_OF_MEMORY && fact.draft && !fact.module); }
            failure_at = 0; source_expansion_destroy(&provider);
            if (path) { assert(unlink(path) == 0); }
            source_manager_destroy(&files); source_arena_destroy(&metadata); finish();
        }
    }
}
