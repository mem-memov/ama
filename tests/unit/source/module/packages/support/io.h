/**
 * @file
 * @brief Control deterministic directory-I/O failures in package-selection tests.
 *
 * Linker wrappers count real directory acquisition, iteration, closure and
 * metadata operations only during one explicitly bracketed selection call.
 * @see [T258 evidence](../../../../../../docs/implementation/versioned-source.md)
 */
#ifndef AMA_TEST_PACKAGE_IO_H
#define AMA_TEST_PACKAGE_IO_H

#include <stddef.h>

/** Begin one measured single-threaded operation; ordinal zero measures without failure.
 * No test-owned directory stream may cross this boundary. Nested scopes are invalid.
 */
void fixture_io_begin(size_t ordinal);

/** Stop injection and return the measured call count, requiring no live directory stream.
 * A simulated close error closes its actual resource before reporting failure.
 */
size_t fixture_io_end(void);

#endif
