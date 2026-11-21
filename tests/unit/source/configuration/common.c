/**
 * @file
 * @brief Supply counted immutable original-byte fixtures to strict INI components.
 * The [configuration contract](../../../../docs/implementation/project-configuration.md)
 * requires every parsed view to retain its source owner until cleanup.
 */
#ifndef AMA_CONFIGURATION_TEST_COMMON
#define AMA_CONFIGURATION_TEST_COMMON
#include "../common.c"

/** Copy exact bytes through the real source owner; embedded NUL and endings remain data. */
static SourceSpan configuration_test_span(const unsigned char *bytes, size_t length) {
    const SourceText *source = NULL;
    assert(source_manager_add(&sources, "package.ini", bytes, length, &source) == SOURCE_OK);
    return (SourceSpan){source, 0, length};
}

/** Construct bounded text fixtures only when their C terminator is not payload. */
static inline SourceSpan configuration_test_text(const char *text) {
    return configuration_test_span((const unsigned char *)text, strlen(text));
}
#endif
