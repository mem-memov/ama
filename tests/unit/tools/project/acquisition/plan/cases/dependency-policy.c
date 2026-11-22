/** @file
 * @brief Verify explicit source-copy dependency membership preserves external bindings and context-only parents.
 * @see [T224 policy](../../../../../../../docs/implementation/source-operations.md)
 */
static size_t dependency_choices;
/** Retain the checked parent target externally; keep every other original edge. */
static bool keep_parent_external(void *context, const DeliverySourceDefinition *owner, const DeliverySourceDependency *edge) {
    (void)context; assert(owner && edge && edge->target); dependency_choices++;
    return strcmp(edge->target, parent_identity) != 0;
}
/** A checked source cycle need not clone the unchanged external parent in source-copy mode. */
static void it_preserves_external_dependency_membership_without_requiring_unchanged_parent_tests(void) {
    prepare(true, false, false, false); dependency_choices = 0;
    AcquisitionClosureProvider provider = {.expand = expand, .admit_test = admit, .follow_dependency = keep_parent_external};
    assert(acquisition_closure_compute(&closure, &child_identity, 1, &provider) == ACQUISITION_CLOSURE_OK);
    assert(closure.valid && expansions == 2 && admissions == 1 && dependency_choices == 2);
    assert(retained("src/parent") && retained("src/parent.ini") && retained("tests/parent/child/check"));
    assert(!retained("src/parent+/extra.data") && !retained("tests/parent/check"));
    const AcquisitionRoleRecord *parent = closure.roles.records;
    while (parent && strcmp(parent->identity, parent_identity)) { parent = parent->next; }
    assert(parent && parent->role == ACQUISITION_CONTEXT && !parent->expanded);
    const DeliverySourceDefinition *child = definitions;
    while (child && strcmp(child->identity, child_identity)) { child = child->next; }
    assert(child && child->dependencies && !strcmp(child->dependencies->target, parent_identity)); finish();
}
