/** @file
 * @brief Derive test-directory pairs from actual source causes and explicit final versions.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) derives original and
 * final associations from logical source names; physical source '+' is irrelevant.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves source/test namespaces, so exceptional destinations never need guesses.
 */
#include "internal.h"
#include "../../../acquisition/closure/tests/association.h"
#include "source/path/logical.h"
#include <string.h>
/** Version tables are exact authorities, not first-match lookup alternatives. */
static bool versions_valid(const SourceVersionPlacement *versions, size_t count) {
    if (count && !versions) { return false; }
    for (size_t i = 0; i < count; i++) {
        ModulePathShape shape;
        if (!versions[i].identity || !versions[i].physical || versions[i].physical[0] != '/'
            || !module_path_describe((const unsigned char *)versions[i].identity,
                strlen(versions[i].identity), &shape)
            || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) { return false; }
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(versions[i].identity, versions[j].identity)) { return false; }
        }
    }
    return true;
}
/** Add one unique source's association, with a destination selected by full version. */
static SourceCopyTestsStatus association(SourceFileMap *files, const SourceCloneMapping *clone,
    const SourceVersionPlacement *versions, size_t count, SourceCopyTestAssociation **head) {
    for (const SourceCopyTestAssociation *pair = *head; pair; pair = pair->next) {
        if (pair->clone == clone) { return SOURCE_COPY_TESTS_OK; }
    }
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)clone->identity, strlen(clone->identity), &shape)
        || shape.extent != MODULE_PATH_MODULE || shape.area != MODULE_PATH_SRC) { return SOURCE_COPY_TESTS_PATH; }
    const SourceVersionPlacement *version = NULL;
    for (size_t i = 0; i < count; i++) {
        if (strlen(versions[i].identity) == shape.version_length
            && !memcmp(versions[i].identity, clone->identity, shape.version_length)) { version = &versions[i]; }
    }
    if (!version) { return SOURCE_COPY_TESTS_VERSION; }
    AcquisitionTestDirectory original;
    SourceStatus status = acquisition_test_directory(&files->arena, clone->original, &original);
    if (status != SOURCE_OK) { return status == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_TESTS_MEMORY : SOURCE_COPY_TESTS_PATH; }
    const char *logical = NULL;
    status = acquisition_test_namespace(&files->arena, clone->identity, &logical);
    if (status != SOURCE_OK) { return status == SOURCE_OUT_OF_MEMORY ? SOURCE_COPY_TESTS_MEMORY : SOURCE_COPY_TESTS_PATH; }
    const char *suffix = logical + shape.version_length;
    size_t length = strlen(version->physical);
    const char *destination = source_arena_join(&files->arena, version->physical, "",
        suffix + (version->physical[length - 1] == '/'));
    SourceCopyTestAssociation *pair = destination ? source_arena_allocate(&files->arena, sizeof(*pair)) : NULL;
    if (!pair) { return SOURCE_COPY_TESTS_MEMORY; }
    *pair = (SourceCopyTestAssociation){.next = *head, .clone = clone,
        .original = original.physical, .destination = destination};
    *head = pair;
    return SOURCE_COPY_TESTS_OK;
}
SourceCopyTestsStatus source_copy_test_associations(SourceFileMap *files, const SourceCopyClosure *copy,
    const SourceCloneMap *mains, const SourceVersionPlacement *versions, size_t count,
    const SourceCopyTestAssociation **output, SourceCopyTestsIssue *issue) {
    if (!versions_valid(versions, count)) { return SOURCE_COPY_TESTS_VERSION; }
    SourceCopyTestAssociation *head = NULL;
    for (const AcquisitionMember *member = copy->closure.members.first; member; member = member->next) {
        for (const AcquisitionMemberCause *cause = member->causes; cause; cause = cause->next) {
            if (cause->kind != ACQUISITION_TEST_MEMBER) { continue; }
            *issue = (SourceCopyTestsIssue){.member = member, .cause = cause};
            const SourceCloneMapping *clone = mains->first;
            while (clone && strcmp(clone->original->logical_path, cause->requester->identity)) { clone = clone->next; }
            issue->clone = clone;
            if (!clone) { return SOURCE_COPY_TESTS_MAIN; }
            SourceCopyTestsStatus status = association(files, clone, versions, count, &head);
            if (status != SOURCE_COPY_TESTS_OK) { return status; }
        }
    }
    *output = head;
    return SOURCE_COPY_TESTS_OK;
}
