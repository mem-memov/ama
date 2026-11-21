/** @file
 * @brief Share the required-version scope cache between module preparation and diagnostics.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) normalizes authored paths.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) retains test ancestry.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) isolates standard origins.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) makes each retained version
 * the sole package alias authority. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps prepared and failed scratch under the Project arena destructor.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) selects members
 * only from complete normalized operands; [PKG-09](../../../../spec/project/packages.md#pkg-09)
 * applies the same source/test policy to their namespaces.
 */
#ifndef AMA_PROJECT_CONFIGURATION_INTERNAL_H
#define AMA_PROJECT_CONFIGURATION_INTERNAL_H
#include "project/dependencies/internal.h"

/** One arena-owned normalized module endpoint and optional qualified member.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) keeps member
 * selection distinct from the original source operand and loaded namespace.
 */
typedef struct {
    const ModulePathValue *module;
    const ModuleConstantPath *constant;
} ProjectPathResolution;

/** One complete immutable package scope keyed by retained source-owner identity.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids merging versions
 * by short names; successful publication occurs only after complete parsing.
 */
struct ProjectAliasPackage {
    ProjectAliasPackage *next;
    const ModuleVersion *version;
    const ConfigurationAliasScope *scope;
    const DependencySelection *selection;
};
/** Return or prepare this required module's exact package scope; no filesystem work.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) supplies retained bytes;
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) fixes the logical version base.
 */
bool project_package_aliases(Project *, ProjectModule *, const ConfigurationAliasScope **, const DependencySelection **);
/** Build one exact selected catalog view with retained physical test ancestry.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) uses actual availability;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) preserves structural provenance.
 */
bool project_configuration_view(Project *, ProjectModule *);
/** Compile this version's selectors against the shared view without loading targets.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) fixes exact selected-unit coverage.
 */
bool project_package_selection(Project *, ProjectModule *, const ConfigurationDocument *,
    const ConfigurationAliasScope *, const DependencySelection **);
/** Normalize a written dependency in its own immutable defining scope.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) retains the raw
 * source span; [PKG-05](../../../../spec/project/packages.md#pkg-05) isolates standard origins.
 */
bool project_dependency_path(Project *, ProjectModule *, SourceSpan, ProjectPathRole, ProjectPathResolution *);
/** Select the final member of a completely normalized qualified operand.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) requires exact
 * final-component selection without alternate namespace probes. Output publishes
 * only on success; failed arena scratch remains owned by the project.
 */
bool project_constant_path(Project *, ProjectModule *, const ModulePathValue *, const ModuleConstantPath **);
/** Check the selected complete namespace against defining-package permissions.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) and
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) apply equally to ordinary
 * declarations and qualified constant namespaces, retaining the raw request site.
 */
bool project_dependency_endpoint(Project *, ProjectModule *, SourceSpan, const ModulePathValue *);
#endif
