/** @file
 * @brief Check combined original graph, known input, explicit client and standard-root review boundaries.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Admission requires the same original view/domain; invalid late roots cannot publish a partial review. */
static void it_refuses_unadmitted_domains_and_keeps_review_output_atomic_on_invalid_clients(void) {
    begin_removal(MATERIAL_NONE, "generated/project/dependencies/vendor/pkg/v1.0.0/src");
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_DOMAIN);
    assert(removal.domain_status == SOURCE_TREE_DOMAIN_UNSELECTED_VERSION && !removal.valid);
    end_removal(MATERIAL_NONE);
    begin_removal(MATERIAL_TESTS, main_path);
    const ModuleSnapshotEntry *saved = analysis.view.entries; analysis.view.entries = NULL;
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_INVALID && !removal.attempted);
    analysis.view.entries = saved;
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    DeliverySourceDefinition foreign = *definition("/vendor/pkg/v1.0.0/src/parent/child");
    SourceClientRoot root = {.definition = &foreign};
    SourceClientRootBlocker blocker = {0};
    SourceRemovalReview review = {.clients = &blocker};
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, &root, &review) == SOURCE_REMOVAL_INVALID);
    assert(review.clients == &blocker && !review.consumers);
    assert(source_removal_selection_review(NULL, NULL, NULL, NULL, &review) == SOURCE_REMOVAL_INVALID);
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, NULL) == SOURCE_REMOVAL_INVALID);
    end_removal(MATERIAL_TESTS);
}
/** Known native/configuration inputs are protected automatically without caller-duplicated use records. */
static void it_protects_original_native_and_configuration_inputs_without_treating_guidance_as_compilation(void) {
    const char *suffixes[] = {".c", ".h", ".abi", ".cmake", ".ini"};
    for (size_t choice = 0; choice < sizeof(suffixes) / sizeof(*suffixes); choice++) {
        begin_removal(MATERIAL_NATIVE_ASSET, main_path);
        const SourceCapturedUnit *native = unit("/vendor/pkg/v1.0.0/src/native");
        const AcquisitionUnitEntry *file = native->files; size_t stem = strlen(source_path(native->source->text));
        while (file && strcmp(file->file.path + stem, suffixes[choice])) { file = file->next; }
        assert(file); AcquisitionUnitEntry input = *file; input.next = NULL;
        SourceDeclaredGroup declared = {.requester = unit("/vendor/pkg/v1.0.0/src/parent/child"), .files = &input};
        assert(source_removal_selection_build(&removal, &selection, &analysis, &declared) == SOURCE_REMOVAL_SELECTION_OK);
        SourceRemovalReview review;
        assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
        assert(review.consumers && review.consumers->next && !review.consumers->next->next);
        assert(review.consumers->next->kind == SOURCE_REMOVAL_INPUT
            && !strcmp(review.consumers->next->owner->identity, "/vendor/pkg/v1.0.0/src/native"));
        for (const SourceRemovalInputUse *use = removal.inputs; use; use = use->next) {
            assert(!strstr(use->path, "/README.md"));
        }
        end_removal(MATERIAL_NATIVE_ASSET);
    }
}
/** Global declarations preserve shared outside ownership even without manually supplied review inputs. */
static void it_derives_shared_original_declaration_uses_for_surviving_owners(void) {
    begin_removal(MATERIAL_TESTS, main_path); AcquisitionUnitEntry input; asset(&input, "shared declared asset\n");
    SourceDeclaredGroup outside = {.requester = unit("/vendor/pkg/v1.0.0/src/parentish"), .files = &input};
    SourceDeclaredGroup selected = {.next = &outside,
        .requester = unit("/vendor/pkg/v1.0.0/src/parent/child"), .files = &input};
    assert(source_removal_selection_build(&removal, &selection, &analysis, &selected) == SOURCE_REMOVAL_SELECTION_OK);
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
    assert(review.consumers && review.consumers->next && !review.consumers->next->next);
    assert(review.consumers->next->kind == SOURCE_REMOVAL_INPUT
        && !strcmp(review.consumers->next->owner->identity, "/vendor/pkg/v1.0.0/src/parentish"));
    end_removal(MATERIAL_TESTS);
}
/** A selected configuration asset cannot remove the version environment of surviving definitions. */
static void it_protects_the_selected_package_environment_for_all_surviving_original_modules(void) {
    begin_removal(MATERIAL_TESTS, main_path);
    const SourceCapturedUnit *child = unit("/vendor/pkg/v1.0.0/src/parent/child");
    const SourceText *configuration = child->source->version->configuration;
    AcquisitionUnitEntry input = {.file = {.path = source_path(configuration), .content = configuration,
        .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    SourceDeclaredGroup declared = {.requester = child, .files = &input};
    assert(source_removal_selection_build(&removal, &selection, &analysis, &declared) == SOURCE_REMOVAL_SELECTION_OK);
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
    size_t modules = 0, inputs = 0;
    for (const SourceRemovalBlocker *block = review.consumers; block; block = block->next) {
        if (block->kind == SOURCE_REMOVAL_MODULE) { modules++; }
        else { assert(!strcmp(block->target, source_path(configuration))); inputs++; }
    }
    assert(modules == 1 && inputs == 2); end_removal(MATERIAL_TESTS);
}
/** Explicit standard roots have no invented package.ini owner or tests namespace. */
static void it_reviews_standard_material_without_invented_ordinary_configuration_requirements(void) {
    begin_removal(MATERIAL_STANDARD, subtree_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 1 && !strcmp(removal.identities[0], "//child"));
    for (const SourceRemovalInputUse *input = removal.inputs; input; input = input->next) {
        assert(strcmp(input->owner->identity, "//child"));
    }
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal.arena, &removal, NULL, NULL, &review) == SOURCE_REMOVAL_OK);
    end_removal(MATERIAL_STANDARD);
}
