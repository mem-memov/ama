/** @file
 * @brief Compose immutable relocation plans and count real descriptor ownership for application preflight tests.
 */
#ifndef AMA_RELOCATION_TRANSACTION_PREFLIGHT_TEST_COMMON
#define AMA_RELOCATION_TRANSACTION_PREFLIGHT_TEST_COMMON
#include <dirent.h>
#include "../../common.c"
#include "tools/relocation/transaction/internal.h"

static RelocationReferences references;
static RelocationStaging staging;
static ModuleRoot roots[2];
static RelocationRequest move_request;

/** Count open descriptors through the Linux descriptor directory without reading their target contents. */
static size_t descriptors(void) {
    DIR *directory = opendir("/proc/self/fd");
    assert(directory != NULL);
    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0) { count++; }
    }
    assert(closedir(directory) == 0);
    return count;
}
/** Build a real original/final source graph before any lease exists; the outside importer is deliberately unused. */
static void prepare(const char *name) {
    setup(name);
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old"); package("imports", "imports"); folder("recovery1"); folder("recovery2");
    file("project/packages/test/fixture/v1.0.0/src/old/a","@!\n");
    file("imports/packages/test/imports/v1.0.0/src/unused","@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
    roots[0] = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    roots[1] = (ModuleRoot){.path=path("imports"),.kind=MODULE_ROOT_PROJECT};
    move_request = (RelocationRequest){roots,2,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(relocation_preview_build(&preview,&move_request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references}));
    assert(module("/test/fixture/v1.0.0/src/old/a") != NULL);
}
/** Retained application preflight can never authorize a source move merely by being destroyed. */
static void done(void) {
    untouched(&move_request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    finish();
}
/** Initialize an application owner over the immutable fixture plan without invoking publication. */
static void application(RelocationTransaction *transaction) {
    relocation_transaction_init(transaction,sources.allocator);
    transaction->staging = &staging;
}
#endif
