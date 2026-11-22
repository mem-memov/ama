/** @file
 * @brief Share explicit original input ownership between source selection and copy expansion.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) requires declared assets,
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps module data roles visible, and
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * protects shared surviving consumers. No ownership is guessed from filenames.
 */
#ifndef AMA_PROJECT_SOURCE_DECLARED_INPUTS_H
#define AMA_PROJECT_SOURCE_DECLARED_INPUTS_H
#include "../capture/capture.h"
/** Borrow an exact original unit requester and complete stable verified physical
 * input captures through operation teardown. Keys are normalized absolute paths;
 * no C/CMake inspection or adjacency establishes this relation. A captured input
 * may itself be an indexed main, requiring complete unit/test participation.
 * version is an exact original selected origin for version-local files; NULL
 * explicitly denotes unversioned inputs. A bound group's keys remain inside that
 * version. This origin is supplied explicitly, never guessed from filenames.
 * The consuming operation decides inclusion; declarations alone grant no mutation.
 */
typedef struct SourceDeclaredGroup {
    struct SourceDeclaredGroup *next;
    const SourceCapturedUnit *requester;
    const ModuleVersion *version;
    const AcquisitionUnitEntry *files;
} SourceDeclaredGroup;
/** Validate exact original handle membership before dereference and PRESENT/FOUND
 * absolute-key file/directory shapes for every nonempty group. Optional version
 * handles must belong to the same capture and contain every supplied key. NULL list is valid
 * over a valid capture. No allocation/I/O/normalization or freshness proof occurs.
 * O(G*(U+V)+F) visits plus bound-path comparisons for G groups, U original units,
 * V selected versions and F captured members.
 */
bool source_declared_groups_valid(const SourceRootCapture *, const SourceDeclaredGroup *);
/** After shape admission, compare every supplied observation that shares a key
 * with an original main/companion or selected version configuration. Unknown
 * opaque assets keep their explicit captured observation. Complete original dot
 * companion capture also certifies missing members; supplied new companions
 * refuse. Known roots, main ancestors and required version/area directories
 * cannot become regular files.
 * This is a consistency check, not complete directory-membership/freshness proof.
 * On a byte/kind contradiction report its borrowed key; invalid inputs report
 * false without a key. O(F*(S+V+R)) key comparisons plus validation and matched
 * bytes, for S originally captured unit files and R roots. No allocation.
 */
bool source_declared_groups_match_original(const SourceRootCapture *, const SourceDeclaredGroup *, const char **failed_path);
#endif
