/** @file
 * @brief Resolve exact physical operands independently of logical module naming.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects aliases;
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes existing sources from explicitly allowed missing destinations;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * leaves transaction freshness to later validation;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed path scratch.
 */
#ifndef AMA_SOURCE_PATH_PHYSICAL_H
#define AMA_SOURCE_PATH_PHYSICAL_H
#include <stdbool.h>
#include "source/memory/arena.h"
#include "../exact.h"
/** Storage covers allocation or representable-capacity refusal; no guessed path
 * limit is imposed under [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
typedef enum { SOURCE_PHYSICAL_OK, SOURCE_PHYSICAL_INVALID, SOURCE_PHYSICAL_MISSING,
    SOURCE_PHYSICAL_UNSUPPORTED, SOURCE_PHYSICAL_IO_ERROR, SOURCE_PHYSICAL_STORAGE } SourcePhysicalStatus;
/** Roots require directories; endpoints may also be regular files under
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01).
 */
typedef enum { SOURCE_PHYSICAL_DIRECTORY, SOURCE_PHYSICAL_ENDPOINT } SourcePhysicalKind;
/** Absolute arena-lifetime key and observed metadata, not a filesystem lease.
 * Missing final paths have zero metadata. directory_required preserves explicit
 * directory intent for the later creator under
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01).
 */
typedef struct {
    const char *path;
    struct stat metadata;
    bool present, directory_required;
} SourcePhysicalPath;
/** Arena-owned failure path when available, with exact child-probe cause under
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 */
typedef struct { SourcePhysicalStatus status; const char *path; PathProbe probe; } SourcePhysicalIssue;
/** Resolve one explicit byte path without source reads, creation or symlink
 * following. Borrow input for this call; result/scratch live in arena. Verify
 * each ordinary component before interpreting a following '..'. Intermediate
 * components must exist as directories. Only an explicitly allowed missing final
 * component succeeds; a trailing slash requires a directory. Preserve raw root
 * prefix bytes; logical module/name validation belongs to the catalog. Require
 * stable traversed directories during this synchronous observation. Assign output
 * only on success and the supplied issue on every call; output and issue are distinct objects.
 * No transaction freshness certificate is issued under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
SourcePhysicalStatus source_physical_resolve(SourceArena *, const char *, SourcePhysicalKind,
    bool missing_final, SourcePhysicalPath *output, SourcePhysicalIssue *issue);
/** Normalize an explicit creation destination over the same exact verified
 * prefix as source_physical_resolve, admitting multiple missing ordinary tail
 * components. Do not create, read source bytes, follow aliases, repair a regular
 * parent or cancel an unverified missing component through '.'/'..'. Navigation
 * in the actually existing prefix retains ordinary strict traversal semantics.
 * On success return either the genuine existing endpoint or a normalized
 * absolute proposal with present=false/zero metadata. Kind and trailing slash
 * preserve directory intent. The proposal grants no namespace, root-setting,
 * absence/freshness or mutation authority; final image/topology proof remains
 * mandatory under [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * and [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 * Preserve output on failure; arena owns scratch and retained diagnostic keys.
 * No fixed path/depth cap. Beyond shared finite prefix-walk bounds, tail time
 * and owned buffer are O(input bytes). Inputs/outcomes obey the same lifetime
 * and distinct output/issue requirements as source_physical_resolve.
 */
SourcePhysicalStatus source_physical_destination(SourceArena *, const char *, SourcePhysicalKind,
    SourcePhysicalPath *output, SourcePhysicalIssue *issue);
#endif
