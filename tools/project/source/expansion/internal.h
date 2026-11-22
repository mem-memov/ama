/** @file
 * @brief Share narrow structural-fact preparation operations.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) separates units,
 * selected-version metadata and ancestor context. Scratch remains owner-local.
 */
#ifndef AMA_PROJECT_SOURCE_EXPANSION_INTERNAL_H
#define AMA_PROJECT_SOURCE_EXPANSION_INTERNAL_H
#include "expansion.h"
/** Intern one selected prepared version and retain its exact metadata once. */
typedef struct SourceExpansionVersion {
    struct SourceExpansionVersion *next;
    const ModuleVersion *version;
    AcquisitionVersionSource source;
    const AcquisitionUnitEntry *configuration, *guidance;
} SourceExpansionVersion;
/** Capture raw files and compare every covered original indexed unit observation. */
AcquisitionClosureStatus source_expansion_files(SourceExpansion *, const ModuleSource *, const AcquisitionUnitEntry **);
/** Compare indexed units within an exact directory or module structural scope. */
AcquisitionClosureStatus source_expansion_observed(SourceExpansion *, const AcquisitionUnitEntry *, const char *, bool module);
/** Prepare immutable selected-version configuration/guidance. NULL is standard. */
AcquisitionClosureStatus source_expansion_version(SourceExpansion *, const ModuleVersion *, AcquisitionExpansion *);
/** Enumerate original logical descendants and required retained ancestor mains. */
AcquisitionClosureStatus source_expansion_topology(SourceExpansion *, const ModuleSource *, AcquisitionExpansion *);
/** Capture one selected version's complete source root, including an empty root. */
AcquisitionClosureStatus source_expansion_scope(SourceExpansion *, const ModuleVersion *, AcquisitionExpansion *);
/** Fill initially empty declaration fields with explicit owned input groups and
 * exact indexed data mains. No outgoing dependency or guessed filename establishes a declaration.
 */
AcquisitionClosureStatus source_expansion_declared(SourceExpansion *, const ModuleSource *, AcquisitionExpansion *);
#endif
