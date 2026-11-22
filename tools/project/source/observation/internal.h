/** @file
 * @brief Separate original observation records, physical reads and consistency admission.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps complete observations and failure witnesses owned without recapture fallback.
 */
#ifndef AMA_PROJECT_SOURCE_OBSERVATION_INTERNAL_H
#define AMA_PROJECT_SOURCE_OBSERVATION_INTERNAL_H
#include "observation.h"
/** Mutable only during preparation; immutable image rows copy these values at seal. */
typedef struct SourceObservedEntry {
    struct SourceObservedEntry *next;
    SourceCatalogImageEntry value;
    const char **names;
    size_t name_count;
} SourceObservedEntry;
/** Find a captured or unavailable named entry by exact physical key. */
SourceObservedEntry *source_observation_find(SourceObservedImage *, const char *);
/** Retain one named placeholder and owned key; equal keys are idempotent. */
SourceObservedEntry *source_observation_entry(SourceObservedImage *, const char *);
/** Preserve first owner failure and map it to the common catalog's diagnostic. */
ModuleStatus source_observation_fail(SourceObservedImage *, ModuleCatalog *, SourceObservationStatus, ModuleStatus, const char *);
/** Record exact metadata; contradictory repeated identity/type observations refuse. */
ModuleStatus source_observation_metadata(SourceObservedImage *, ModuleCatalog *, const char *, const struct stat *);
/** Root/child/list/read callbacks capture local observations only before sealing. */
ModuleStatus source_observation_root(void *, ModuleCatalog *, const char *, struct stat *);
/** Complete parent membership precedes exact metadata probing. */
ModuleStatus source_observation_child(void *, ModuleCatalog *, const char *, const char *, const char *, struct stat *);
/** Retain complete names once and return stable cached directory snapshots. */
ModuleStatus source_observation_entries(void *, ModuleCatalog *, const char *, const char ***, size_t *);
/** Read exact local bytes once into this owner, with no content guessing. */
SourceStatus source_observation_read(void *, const char *, const SourceText **);
/** Compare common selected sources, metadata and package/alias bytes with original capture. */
SourceObservationStatus source_observation_compare(SourceObservedImage *);
/** Complete every original companion and refuse newly listed same-module members. */
SourceObservationStatus source_observation_units(SourceObservedImage *);
/** Materialize finite rows and seal the original image after all comparisons pass. */
SourceObservationStatus source_observation_seal(SourceObservedImage *);
#endif
