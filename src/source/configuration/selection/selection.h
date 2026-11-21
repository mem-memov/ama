/**
 * @file
 * @brief Share actual prepared identities and sparse selected units across dependency checks.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) unions actual matches;
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains structural units;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) associates tests;
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) checks authored edges.
 * [SRC-05](../../../../spec/language/source-and-names.md#src-05) preserves the
 * distinction between ordinary physical directories and structural plus directories.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) makes borrowing
 * explicit; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * bounds storage; [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves original preparation failures.
 */
#ifndef AMA_SOURCE_CONFIGURATION_SELECTION_H
#define AMA_SOURCE_CONFIGURATION_SELECTION_H
#include "source/configuration/selectors/selector.h"
#include "source/path/logical.h"

/** Partial preparation is never published under [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
typedef enum { DEPENDENCY_SELECTION_OK, DEPENDENCY_SELECTION_INVALID,
    DEPENDENCY_SELECTION_LIMIT, DEPENDENCY_SELECTION_OUT_OF_MEMORY } DependencySelectionStatus;
/** Actual canonical endpoint plus its structural version/area boundaries.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) requires actual eligible records, not guessed ancestors.
 */
typedef struct {
    const ModulePathValue *identity;
    ModulePathShape shape;
    size_t test_directory_prefix;
} DependencyViewEntry;
/** Verified physical-layout projection for one actual endpoint.
 * For a test module, test_directory_prefix counts leading ordinary directory
 * components after tests/, stopping before its first structural name+/ ancestor
 * or its main filename. Other endpoints use zero. [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * distinguishes mirrored ordinary test directories from test-module subtrees.
 * The source-view producer derives this from verified physical provenance; a
 * logical path alone cannot recover the discarded structural plus markers.
 */
typedef struct { const ModulePathValue *identity; size_t test_directory_prefix; } DependencyViewInput;
/** Shared sorted records borrow immutable normalized identity owners, not the input pointer array.
 * The caller supplies the effective prepared PKG-02 view, including empty versions.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) matching does not create availability.
 */
typedef struct { const DependencyViewEntry *entries; size_t count; } DependencyView;
/** One actual root selected once, irrespective of how many masks match it.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) defines a set of identities.
 */
typedef struct DependencySelectionRoot {
    const DependencyViewEntry *entry;
    const struct DependencySelectionRoot *next;
} DependencySelectionRoot;
/** Sparse root union borrowing the shared view, which outlives this object.
 * Patterns and alias tables need survive construction only.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) separates allowed selections from eager loading.
 */
typedef struct {
    const DependencyView *view;
    const DependencyViewEntry *owner;
    const DependencySelectionRoot *roots;
    size_t count;
} DependencySelection;
/** Explicit-edge decisions distinguish policy refusal from invalid origin metadata.
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) precedes same-version permission.
 */
typedef enum { DEPENDENCY_EDGE_ALLOWED, DEPENDENCY_EDGE_UNDECLARED,
    DEPENDENCY_EDGE_SOURCE_TEST, DEPENDENCY_EDGE_INVALID } DependencyEdgeDecision;

/** Validate unique actual ordinary versions/modules and their version membership.
 * Arena/output/issue are valid. Identities are immutable successful normalized
 * values that outlive the view; count zero permits NULL input. No I/O occurs.
 * Failure preserves output; partial preparation stays arena-owned.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) restricts eligible endpoints.
 */
DependencySelectionStatus dependency_view_build(SourceArena *arena,
    const DependencyViewInput *identities, size_t count, const DependencyView **output, SourceIssue *issue);
/** Project a verified area-relative physical module path onto exact test provenance.
 * Physical bytes exclude src/ or tests/ and remain borrowed only for the call.
 * Identity is a normalized ordinary complete module. Catalog discovery already
 * proved filesystem topology, spelling and ancestor main-file presence; this
 * operation checks the exact logical/physical component correspondence, without I/O.
 * No allocation; failure preserves output. [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * identifies structural plus ancestors; [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * requires the ordinary-directory prefix for associated tests.
 */
DependencySelectionStatus dependency_view_provenance(const ModulePathValue *identity,
    const unsigned char *physical, size_t length, DependencyViewInput *output, SourceIssue *issue);
/** Select roots for one actual version present in view. Input pattern pointers are
 * successful T263 results; zero count permits NULL. The view/identity owners
 * outlive selection. Failure preserves output; arena retains partial storage.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) unions all actual matches.
 */
DependencySelectionStatus dependency_selection_build(SourceArena *arena, const DependencyView *view,
    const ModulePathValue *owner, const DependencySelector *const *patterns, size_t count,
    const DependencySelection **output, SourceIssue *issue);
/** Decide coverage of an actual view member without allocation, importing or filesystem probes.
 * Target is an immutable normalized value. Missing/ineligible metadata returns false.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) covers complete units;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) adds associated directories.
 */
bool dependency_selection_covers(const DependencySelection *selection, const ModulePathValue *target);
/** Check an explicitly authored ordinary-definition edge against its owner's table.
 * Source/target must be complete module identities and source must belong to owner.
 * Caller-supplied Type substitutions are not newly authored receiving edges and
 * must not be submitted a second time. Permission does not prove availability.
 * No allocation, source loading or state mutation. [PKG-09](../../../../spec/project/packages.md#pkg-09)
 * blocks src-to-tests before [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * grants same-version/standard privileges or external selected-unit coverage.
 */
DependencyEdgeDecision dependency_edge_check(const DependencySelection *selection,
    const ModulePathValue *source, const ModulePathValue *target);
#endif
