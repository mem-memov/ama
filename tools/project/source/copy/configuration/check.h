/** @file
 * @brief Verify independently reparsed destination permissions equal their prepared exact set.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves selected root sets plus only explicit authorized additions;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires final admission rather than trusting intended text edits.
 */
#ifndef AMA_SOURCE_COPY_CONFIGURATION_CHECK_H
#define AMA_SOURCE_COPY_CONFIGURATION_CHECK_H
#include "prepare.h"
/** Distinguish foreign preparation, wrong destination and exact missing/extra roots. */
typedef enum { SOURCE_CONFIGURATION_CHECK_OK, SOURCE_CONFIGURATION_CHECK_INVALID,
    SOURCE_CONFIGURATION_CHECK_SCOPE, SOURCE_CONFIGURATION_CHECK_MISSING,
    SOURCE_CONFIGURATION_CHECK_EXTRA } SourceConfigurationCheckStatus;
/** Borrow first missing expected or unexpected actual selected root. */
typedef struct { const DependencyViewEntry *expected, *actual; } SourceConfigurationCheckIssue;
/** Require actual scopes owned by two valid immutable environments and canonical
 * unique expected-root pointers into the preparation environment's actual view.
 * Final scope must have identical complete version identity, physical version and
 * configuration path. Compare root sets by canonical names across independent
 * view owners, not by pointer equality or broad coverage. No allocation/I/O,
 * reparsing, permissions or source mutation. Reset issue on entry; return first
 * missing then first extra witness. Finite scope/view/root scans terminate;
 * O(S + R*V + R² + R*A) comparisons, constant scratch, where S scopes, V expected
 * view entries, R requested roots, A actual roots. Inputs outlive diagnostics.
 * The caller must independently rebuild final environment from prepared bytes
 * and check every affected configuration; prepared roots must come from the
 * authorized prepare/initialize operation. Success proves set equality only,
 * not that a caller-invented set or source mutation was authorized.
 */
SourceConfigurationCheckStatus source_copy_configuration_check(
    const SourcePackageEnvironment *, const SourcePackageScope *, const SourceSelectionRoots *,
    const SourcePackageEnvironment *, const SourcePackageScope *, SourceConfigurationCheckIssue *);
#endif
