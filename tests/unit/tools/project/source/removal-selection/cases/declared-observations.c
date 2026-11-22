/** @file
 * @brief Verify shared declared-input shape, version-origin and frozen observation admission.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Malformed global declarations refuse before any material group is published. */
static void it_refuses_unowned_or_malformed_declared_input_observations_before_material_expansion(void) {
    for (unsigned choice = 0; choice < 12; choice++) {
        begin_removal(MATERIAL_NONE, main_path); AcquisitionUnitEntry entry; asset(&entry, "asset\n");
        const SourceCapturedUnit *owner = unit("/vendor/pkg/v1.0.0/src/parent/child");
        SourceDeclaredGroup group = {.requester = owner, .version = owner->source->version, .files = &entry};
        ModuleVersion foreign = *owner->source->version;
        if (choice == 0) { group.files = NULL; }
        if (choice == 1) { entry.file.path = NULL; }
        if (choice == 2) { entry.file.path = ""; }
        if (choice == 3) { entry.file.path = "relative/asset"; }
        if (choice == 4) { entry.file.state = DELIVERY_FILE_ABSENT; }
        if (choice == 5) { entry.file.issue = PATH_IO_ERROR; }
        if (choice == 6) { entry.file.content = NULL; }
        if (choice == 7) { entry.directory = true; }
        if (choice == 8) { group.requester = NULL; }
        if (choice == 9) { group.version = &foreign; }
        if (choice == 10) {
            entry.file.path = source_arena_join(&selection.arena, owner->source->version->physical_path, "ish", "/asset"); assert(entry.file.path);
        }
        if (choice == 11) { entry.file.path = "/outside/asset"; }
        assert(!source_declared_groups_valid(&capture, &group));
        assert(source_removal_selection_build(&removal, &selection, &analysis, &group) == SOURCE_REMOVAL_SELECTION_INVALID);
        assert(!removal.valid && !removal.groups); end_removal(MATERIAL_NONE);
    }
    begin_removal(MATERIAL_NONE, main_path);
    SourceRootCapture unbuilt = {0}; const char *failed = "sentinel";
    assert(!source_declared_groups_valid(NULL, NULL) && !source_declared_groups_valid(&unbuilt, NULL));
    assert(source_declared_groups_valid(&capture, NULL));
    assert(!source_declared_groups_match_original(NULL, NULL, &failed) && !failed);
    assert(!source_declared_groups_match_original(&capture, NULL, NULL));
    assert(source_declared_groups_match_original(&capture, NULL, &failed) && !failed);
    end_removal(MATERIAL_NONE);
}
/** Known mains, opaque companions and package metadata cannot be recaptured with contradictory bytes/kind. */
static void it_refuses_declared_observations_that_change_known_original_main_companion_or_configuration_bytes(void) {
    for (unsigned choice = 0; choice < 4; choice++) {
        begin_removal(MATERIAL_NONE, main_path);
        const SourceCapturedUnit *owner = unit("/vendor/pkg/v1.0.0/src/parent/child");
        const SourceCapturedUnit *selected = unit(choice == 1 ? "/vendor/pkg/v1.0.0/src/parent" : "/vendor/pkg/v1.0.0/src/parentish");
        const AcquisitionUnitEntry *original = selected->files;
        const char *key = choice == 1 ? selected->source->configuration_path : source_path(selected->source->text); assert(key);
        while (original && strcmp(original->file.path, key)) { original = original->next; }
        assert(original);
        AcquisitionUnitEntry entry = *original; entry.next = NULL;
        if (choice == 2) {
            const SourceText *configuration = owner->source->version->configuration;
            entry = (AcquisitionUnitEntry){.file = {.path = source_path(configuration), .content = configuration,
                .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
        }
        SourceDeclaredGroup group = {.requester = owner, .version = owner->source->version, .files = &entry};
        const char *failed = "sentinel";
        assert(source_declared_groups_valid(&capture, &group));
        assert(source_declared_groups_match_original(&capture, &group, &failed) && !failed);
        if (choice == 3) { entry.directory = true; entry.file.content = NULL; }
        else {
            assert(source_manager_add(&selection.files, entry.file.path, (const unsigned char *)"changed observation", 19, &entry.file.content) == SOURCE_OK);
        }
        assert(source_declared_groups_valid(&capture, &group));
        assert(!source_declared_groups_match_original(&capture, &group, &failed) && failed == entry.file.path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, &group) == SOURCE_REMOVAL_SELECTION_CHANGED);
        assert(!removal.valid && !removal.groups && removal.failed_path == entry.file.path);
        end_removal(MATERIAL_NONE);
    }
}
/** Captured companion absence and known directory kinds are observations too. */
static void it_refuses_new_companion_presence_and_known_directory_to_file_contradictions(void) {
    for (unsigned choice = 0; choice < 5; choice++) {
        begin_removal(MATERIAL_NONE, main_path);
        const SourceCapturedUnit *owner = unit("/vendor/pkg/v1.0.0/src/parent/child");
        const char *path = choice == 0 ? source_arena_join(&selection.arena, source_path(owner->source->text), "", ".new")
            : choice == 1 ? owner->source->version->physical_path
            : choice == 2 ? source_arena_join(&selection.arena, owner->source->version->physical_path, "/", "src")
            : choice == 3 ? capture.roots[0].path : "/";
        assert(path); AcquisitionUnitEntry entry = {.file = {.path = path, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
        assert(source_manager_add(&selection.files, path, (const unsigned char *)"new", 3, &entry.file.content) == SOURCE_OK);
        SourceDeclaredGroup group = {.requester = owner, .files = &entry}; const char *failed = NULL;
        assert(source_declared_groups_valid(&capture, &group));
        assert(!source_declared_groups_match_original(&capture, &group, &failed) && failed == path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, &group) == SOURCE_REMOVAL_SELECTION_CHANGED);
        assert(!removal.valid && !removal.groups); end_removal(MATERIAL_NONE);
    }
}
