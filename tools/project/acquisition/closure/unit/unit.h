/** @file
 * @brief Capture a module's exact structural bytes without interpreting its code.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains every same-name
 * companion and the entire active submodule subtree, but no context siblings.
 * [SRC-02](../../../../../spec/language/source-and-names.md#src-02) forbids symlink
 * and directory-identity aliases during capture.
 * Required associated directories share the lossless structural capture boundary.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * also selects complete explicit directory operands.
 */
#ifndef AMA_ACQUISITION_UNIT_H
#define AMA_ACQUISITION_UNIT_H
#include "delivery/native/files/snapshot.h"
#include "../roles/roles.h"
#include "source/path/physical/physical.h"
/** Byte-sorted owned file/directory membership; file bytes belong to the supplied
 * SourceManager. Empty directories are retained; absent optional subtrees are not
 * members under [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
typedef struct AcquisitionUnitEntry {
    struct AcquisitionUnitEntry *next;
    DeliveryFileSnapshot file;
    bool directory;
} AcquisitionUnitEntry;
/** Capture one extensionless main and all basename-dot-nonempty companions below
 * an already verified stable directory. Active requests also retain all entries
 * of the optional basename-plus directory; context requests never enter it.
 * Inputs are borrowed during the call; output paths/records belong to arena and
 * bytes to texts. Main must exist; no symlink/device input or companion directory
 * is admitted. Failure leaves output unchanged and returns an arena-owned failed
 * path when available. No source parsing or native/CMake execution occurs.
 * Entries are sorted bytewise. For R entries and B bytes, sorted insertion costs
 * O(R squared + B), excluding exact filesystem probes. Storage is O(paths+B+R).
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) determines structural
 * retention independently of whether each captured descendant is used by code.
 */
SourceStatus acquisition_unit_capture(SourceArena *, SourceManager *, const char *parent,
    const char *basename, AcquisitionRole, const AcquisitionUnitEntry **output, const char **failed_path);
/** Capture one required associated directory and its entire tree, including the
 * root/empty directories and all raw file bytes. Parent is already verified and
 * stable; name is one exact child, never a relative traversal. Borrow both owners
 * through consumption; publish output only after complete success. A missing
 * directory is an I/O failure, unlike an optional module-plus subtree. The same
 * traversal, ordering, ownership and complexity contract applies as unit capture.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) requires the entire
 * associated directory independently of which concrete test roots are selected.
 */
SourceStatus acquisition_directory_capture(SourceArena *, SourceManager *, const char *parent,
    const char *name, const AcquisitionUnitEntry **output, const char **failed_path);
/** Capture an independently resolved existing directory operand, including a
 * filesystem root without inventing a basename. Borrow its exact path/metadata;
 * require stable verified ancestors and unchanged root device/inode. Recheck the
 * root's regular directory shape, own the root record and traverse exactly as the
 * associated-directory operation. Output changes only on complete success;
 * failures retain scratch and a failed path. No symlink is followed at the root.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits explicit directory SOURCE operands, including empty directories.
 */
SourceStatus acquisition_directory_capture_root(SourceArena *, SourceManager *, const SourcePhysicalPath *,
    const AcquisitionUnitEntry **output, const char **failed_path);
#endif
