/** @file
 * @brief Select one requesting configuration's exact repository route without transport.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) selects the longest
 * whole-component prefix in the requesting environment, never a global table.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps acquisition planning outside the compiler pipeline.
 */
#ifndef AMA_PROJECT_ACQUISITION_ROUTE_H
#define AMA_PROJECT_ACQUISITION_ROUTE_H
#include "source/configuration/document/document.h"
#include "source/path/logical.h"
#include "source/path/normalize/normalize.h"
/** Missing route is distinct from malformed input; neither authorizes fallback. */
typedef enum { ACQUISITION_ROUTE_OK, ACQUISITION_ROUTE_INVALID, ACQUISITION_ROUTE_MISSING } AcquisitionRouteStatus;
/** Borrowed immutable route. Request/source/document owners outlive its use.
 * prefix_length counts bytes from normalized request text, excluding its following
 * slash; shape retains the canonical version boundary. No allocation or I/O. */
typedef struct {
    const ConfigurationDocument *scope;
    const ConfigurationRepository *mapping;
    const ModulePathValue *request;
    ModulePathShape shape;
    size_t prefix_length;
} AcquisitionRoute;
/** Select from one successfully parsed immutable package-profile document and one successful normalized path value.
 * Admit complete canonical ordinary versions/modules only. Output changes only
 * on OK; issue retains the original request span on refusal. An absent mapping
 * returns MISSING. Availability and target configuration are not inputs, so
 * neither can induce fallback or change the scope. */
AcquisitionRouteStatus acquisition_route_select(const ConfigurationDocument *, const ModulePathValue *,
    AcquisitionRoute *output, SourceIssue *issue);
#endif
