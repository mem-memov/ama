/** @file
 * @brief Retain whole associated directories and independently admit concrete direct tests.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) requires direct concrete
 * experiments, keeps support material and avoids recursively requiring tests of tests.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <string.h>
AcquisitionClosureStatus acquisition_closure_tests(AcquisitionClosure *closure, const AcquisitionRoleRecord *record,
    const AcquisitionExpansion *fact, const AcquisitionClosureProvider *provider) {
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)record->identity, strlen(record->identity), &shape)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Invalid active module identity");
    }
    if (shape.area != MODULE_PATH_SRC) {
        if (fact->test_directory || fact->test_files) {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Non-source module cannot introduce an associated test tree");
        }
        return ACQUISITION_CLOSURE_OK;
    }
    AcquisitionTestDirectory expected;
    SourceStatus mapped = acquisition_test_directory(&closure->arena, fact->module, &expected);
    if (mapped != SOURCE_OK) {
        return acquisition_closure_fail(closure, mapped == SOURCE_OUT_OF_MEMORY ? ACQUISITION_CLOSURE_OUT_OF_MEMORY
            : ACQUISITION_CLOSURE_INVALID, "Source/test association failed");
    }
    if (!fact->test_directory || !fact->test_files) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_MISSING_TEST, "Required associated test directory is missing");
    }
    if (!fact->test_directory->logical || !fact->test_directory->physical ||
        strcmp(expected.logical, fact->test_directory->logical) || strcmp(expected.physical, fact->test_directory->physical)) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Associated test directory belongs to a different source module");
    }
    const AcquisitionTestRoot *roots = NULL;
    AcquisitionTestRootsStatus selected = acquisition_test_roots(&closure->arena, fact->definitions, expected.logical, &roots);
    if (selected != ACQUISITION_TEST_ROOTS_OK) {
        return acquisition_closure_fail(closure, selected == ACQUISITION_TEST_ROOTS_OUT_OF_MEMORY ? ACQUISITION_CLOSURE_OUT_OF_MEMORY
            : selected == ACQUISITION_TEST_ROOTS_MISSING ? ACQUISITION_CLOSURE_MISSING_TEST : ACQUISITION_CLOSURE_INVALID,
            "Associated directory has no admissible concrete direct test");
    }
    size_t admitted = 0;
    for (const AcquisitionTestRoot *test = roots; test; test = test->next) {
        SourceIssue issue = {0};
        AcquisitionClosureStatus status = provider->admit_test(provider->context, test->definition, &issue);
        if (status == ACQUISITION_CLOSURE_OPEN_TEST) { continue; }
        if (status != ACQUISITION_CLOSURE_OK) {
            closure->issue = issue;
            return acquisition_closure_fail(closure, status, "Independent concrete test admission failed");
        }
        admitted++;
        AcquisitionTestRoot *retained = source_arena_allocate(&closure->arena, sizeof(*retained));
        if (!retained) { return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_OUT_OF_MEMORY, "Test selection metadata allocation failed"); }
        *retained = (AcquisitionTestRoot){.definition = test->definition};
        AcquisitionTestRoot **position = &closure->tests;
        while (*position && strcmp((*position)->definition->identity, test->definition->identity) < 0) { position = &(*position)->next; }
        retained->next = *position; *position = retained;
    }
    if (!admitted) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_MISSING_TEST, "Associated directory contains only open test candidates");
    }
    AcquisitionClosureStatus status = acquisition_closure_files(closure, fact->test_files, record, fact->source, ACQUISITION_TEST_MEMBER);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    size_t length = strlen(expected.logical);
    for (const DeliverySourceDefinition *definition = fact->definitions; definition; definition = definition->next) {
        if (strncmp(definition->identity, expected.logical, length) || definition->identity[length] != '/') { continue; }
        const AcquisitionRoleRecord *helper = NULL;
        status = acquisition_closure_request(closure, definition->identity, ACQUISITION_ACTIVE, ACQUISITION_TEST, record, &helper);
        if (status != ACQUISITION_CLOSURE_OK) { return status; }
    }
    return ACQUISITION_CLOSURE_OK;
}
