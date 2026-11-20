/** @file
 * @brief Share exact importer/outcome teardown across independent disk recovery leaves.
 */
#ifndef AMA_RELOCATION_RECOVERY_TEST_COMMON
#define AMA_RELOCATION_RECOVERY_TEST_COMMON
#include "../report/common.c"
/** Check exact imported module bytes through an independent ordinary source owner. */
static void importer(const char *value) {
    const SourceText *actual;
    assert(source_manager_load(&sources,path("imports/packages/test/imports/v1.0.0/src/unused"),&actual) == SOURCE_OK);
    assert(source_length(actual) == strlen(value) && memcmp(source_bytes(actual),value,strlen(value)) == 0);
}
/** Recovery may complete a committed move, so teardown cannot demand the original source location. */
static void complete_fixture(void) {
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    finish();
}
#endif
