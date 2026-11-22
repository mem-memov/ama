/** @file
 * @brief Describe checked source dependencies without compiler AST or semantic owners.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01) keeps
 * project-tool consumers independent of compiler state; [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * supplies exact resolved names and original request spans.
 * Original source categories feed independent semantic test admission.
 * [PKG-08](../../../spec/project/packages.md#pkg-08)
 */
#ifndef AMA_DELIVERY_SOURCE_RECORDS_H
#define AMA_DELIVERY_SOURCE_RECORDS_H
#include "source/text/source.h"
/** Source-only categories retain original parameterization; exact independent
 * entry inference classifies runtime candidates afterward under [PKG-08](../../../spec/project/packages.md#pkg-08).
 */
typedef enum { DELIVERY_SOURCE_CONCRETE, DELIVERY_SOURCE_PARAMETERIZED,
    DELIVERY_SOURCE_STATIC, DELIVERY_SOURCE_CONSTANTS } DeliverySourceKind;
/** Plain resolved edge with original source evidence. No AST, alias scope,
 * live compiler instance or executable plan crosses the boundary under
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01).
 */
typedef struct DeliverySourceDependency {
    struct DeliverySourceDependency *next;
    const char *target;
    SourceSpan origin;
} DeliverySourceDependency;
/** Plain source-definition export. Identity and edge names belong to the export
 * arena; original source bytes borrow the immutable source owner. Source-only
 * delivery does not manufacture runtime instances under
 * [PKG-08](../../../spec/project/packages.md#pkg-08).
 */
typedef struct DeliverySourceDefinition {
    struct DeliverySourceDefinition *next;
    const char *identity;
    DeliverySourceKind kind;
    DeliverySourceDependency *dependencies;
} DeliverySourceDefinition;
#endif
