/** @file
 * @brief Validate public snapshot membership before inspecting a mutable commit slot.
 * [PKG-06](../../../../../spec/project/packages.md#pkg-06) keeps snapshot state
 * operation-local; foreign handles cannot mutate or borrow another operation.
 */
#include "../internal.h"
AcquisitionSnapshotRecord *acquisition_snapshot_record(AcquisitionSources *sources, const AcquisitionSnapshot *value) {
    for (AcquisitionSnapshotRecord *record = sources->snapshots; record; record = record->next) {
        if (&record->value == value) { return record; }
    }
    return NULL;
}
