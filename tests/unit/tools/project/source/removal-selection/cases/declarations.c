/** @file
 * @brief Check explicit asset ownership and finite module-as-data obligation closure without graph dependency expansion.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Declared ownership, not adjacency, selects an additional asset; surviving shared users still block. */
static void it_includes_selected_explicit_assets_and_preserves_shared_outside_consumers(void) {
    for (unsigned selected_owner = 0; selected_owner < 2; selected_owner++) {
        begin_removal(MATERIAL_TESTS, main_path); AcquisitionUnitEntry file_entry; asset(&file_entry, "declared asset\n");
        SourceDeclaredGroup declared = {.requester = unit(selected_owner
            ? "/vendor/pkg/v1.0.0/src/parent/child" : "/vendor/pkg/v1.0.0/src/parentish"), .files = &file_entry};
        assert(source_removal_selection_build(&removal, &selection, &analysis, &declared) == SOURCE_REMOVAL_SELECTION_OK);
        bool included = false;
        for (size_t index = 0; index < removal.path_count; index++) {
            if (!strcmp(removal.paths[index], file_entry.file.path)) { included = true; }
        }
        assert(included == (selected_owner != 0));
        SourceRemovalInputUse input = {.owner = definition("/vendor/pkg/v1.0.0/tests/outside"), .path = file_entry.file.path};
        SourceRemovalReview review;
        assert(source_removal_selection_review(&removal, &input, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
        assert(review.consumers && (review.consumers->next != NULL) == included);
        end_removal(MATERIAL_TESTS);
    }
}
/** Conflicting/foreign inputs refuse; an explicit module-as-data input still completes its module unit. */
static void it_checks_conflicting_unowned_and_module_role_declared_inputs(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        begin_removal(MATERIAL_NONE, main_path); AcquisitionUnitEntry first, second;
        asset(&first, "first observation\n");
        const SourceCapturedUnit *owner = unit("/vendor/pkg/v1.0.0/src/parent/child");
        SourceCapturedUnit foreign = *owner;
        SourceDeclaredGroup tail = {.requester = owner};
        SourceDeclaredGroup declared = {.requester = owner, .files = &first};
        if (kind == 0) { asset(&second, "changed observation\n"); tail.files = &second; declared.next = &tail; }
        if (kind == 1) { declared.requester = &foreign; }
        if (kind == 2) { declared.files = unit("/vendor/pkg/v1.0.0/src/parentish")->files; }
        assert(source_removal_selection_build(&removal, &selection, &analysis, &declared)
            == (kind == 0 ? SOURCE_REMOVAL_SELECTION_CHANGED : kind == 1 ? SOURCE_REMOVAL_SELECTION_INVALID : SOURCE_REMOVAL_SELECTION_OK));
        assert(removal.valid == (kind == 2));
        if (kind == 2) { assert(removal.identity_count == 3); }
        end_removal(MATERIAL_NONE);
    }
}
/** Explicit module-as-data cycles complete each physical unit once without following semantic dependencies. */
static void it_closes_module_asset_cycles_once_and_leaves_outgoing_dependency_units_untouched(void) {
    begin_removal(MATERIAL_MODULE_ASSET, main_path);
    const SourceCapturedUnit *child = unit("/vendor/pkg/v1.0.0/src/parent/child");
    const SourceCapturedUnit *other = unit("/vendor/pkg/v1.0.0/src/parentish");
    AcquisitionUnitEntry child_main = {.file = {.path = source_path(child->source->text),
        .content = child->source->text, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    AcquisitionUnitEntry other_main = {.file = {.path = source_path(other->source->text),
        .content = other->source->text, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    SourceDeclaredGroup backward = {.requester = other, .files = &child_main};
    SourceDeclaredGroup forward = {.next = &backward, .requester = child, .files = &other_main};
    assert(source_removal_selection_build(&removal, &selection, &analysis, &forward) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 6);
    bool companion = false, nested = false;
    size_t units = 0;
    for (const SourceRemovalGroup *group = removal.groups; group; group = group->next) {
        if (group->kind == SOURCE_REMOVAL_UNIT) { units++; }
    }
    for (size_t index = 0; index < removal.path_count; index++) {
        if (strstr(removal.paths[index], "/parentish.extra")) { companion = true; }
        if (strstr(removal.paths[index], "/parentish+/nested")) { nested = true; }
        assert(!strstr(removal.paths[index], "/remote"));
    }
    assert(units == 6 && companion && nested);
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal, NULL, NULL, &review) == SOURCE_REMOVAL_BLOCKED);
    assert(review.consumers && !review.consumers->next
        && !strcmp(review.consumers->owner->identity, "/vendor/pkg/v1.0.0/tests/outside"));
    end_removal(MATERIAL_MODULE_ASSET);
}
