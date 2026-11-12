/**
 * @file
 * @brief Inject one exact link/rename failure through the test linker's wrappers.
 *
 * Production code has no fault hook. GNU ld --wrap redirects only this leaf's
 * calls; unselected calls execute the real POSIX operation on isolated fixtures.
 */
#ifndef AMA_PUBLICATION_TEST_FAULTS
#define AMA_PUBLICATION_TEST_FAULTS

/** Test-local call observations and one/two explicit failure ordinals. */
static struct {
    bool enabled;
    size_t links;
    size_t renames;
    size_t fail_link;
    size_t fail_rename;
    size_t fail_second_rename;
} publication_faults;

/**
 * Linker-provided original POSIX link used when a publication test has not selected this fault
 * ordinal; source/destination paths stay borrowed through the call.
 */
int __real_link(const char *, const char *);
/**
 * Linker-provided original POSIX rename used outside the selected injected failure; test-owned
 * publication paths may change only under the tested operation.
 */
int __real_rename(const char *, const char *);

/** Fail the selected link before effects; otherwise preserve the real outcome. */
int __wrap_link(const char *source, const char *destination) {
    if (publication_faults.enabled) {
        publication_faults.links++;
        if (publication_faults.links == publication_faults.fail_link) {
            errno = EIO;
            return -1;
        }
    }
    return __real_link(source, destination);
}

/** Permit a second failure to exercise restore failure and backup retention. */
int __wrap_rename(const char *source, const char *destination) {
    if (publication_faults.enabled) {
        publication_faults.renames++;
        if (publication_faults.renames == publication_faults.fail_rename
            || publication_faults.renames == publication_faults.fail_second_rename) {
            errno = EIO;
            return -1;
        }
    }
    return __real_rename(source, destination);
}
#endif
