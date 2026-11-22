/** @file
 * @brief Collect direct runtime test candidates from checked plain source exports.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) distinguishes concrete
 * executable direct roots from open and non-executable support definitions.
 * Internal typed evidence may close original formal parameters at entry admission.
 * [TPL-03](../../../../../spec/language/templates.md#tpl-03)
 */
#ifndef AMA_ACQUISITION_TEST_ROOTS_H
#define AMA_ACQUISITION_TEST_ROOTS_H
#include "delivery/source/records.h"
#include "source/memory/arena.h"
/** Each selected definition remains borrowed from the checked export owner.
 * No runtime instance is created by this [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * experiment selection metadata. */
typedef struct AcquisitionTestRoot {
    struct AcquisitionTestRoot *next;
    const DeliverySourceDefinition *definition;
} AcquisitionTestRoot;
/** Distinguish absent runtime candidates from failed ownership/allocation. */
typedef enum { ACQUISITION_TEST_ROOTS_OK, ACQUISITION_TEST_ROOTS_MISSING,
    ACQUISITION_TEST_ROOTS_INVALID, ACQUISITION_TEST_ROOTS_OUT_OF_MEMORY } AcquisitionTestRootsStatus;
/** Collect byte-sorted immediate ordinary/native candidates in one canonical
 * logical associated directory. Both original concrete and parameterized source
 * definitions participate: exact independent entry analysis must distinguish an
 * internally grounded specialization from a genuinely open template afterward.
 * Require checked plain source exports, with normal definition errors retained.
 * Own list records in arena and borrow exports through consumption; output changes
 * only on success. Missing means no runtime candidate, not final test admission.
 * O(N squared + path comparisons) time and O(candidates) space, with no I/O or
 * execution. [PKG-08](../../../../../spec/project/packages.md#pkg-08) requires
 * subsequent exact entry classification; [TPL-03](../../../../../spec/language/templates.md#tpl-03)
 * permits internal evidence to close original formals. Never infer helper status
 * from filenames, original formal count alone or arbitrary failed diagnostics.
 */
AcquisitionTestRootsStatus acquisition_test_roots(SourceArena *, const DeliverySourceDefinition *,
    const char *logical_directory, const AcquisitionTestRoot **output);
#endif
