/** @file
 * @brief Describe checked source dependencies without compiler AST or semantic owners.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01) keeps
 * project-tool consumers independent of compiler state; [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * supplies exact resolved names and original request spans.
 * Original source categories feed independent semantic test admission.
 * [PKG-08](../../../spec/project/packages.md#pkg-08)
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
 * distinguishes fixed dependency sites from authored paths and preserves
 * normalized constant members during copied-path rewriting.
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04)
 * consumes explicit captured companions without probing staged future paths.
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
    /** True for a requiring construct rather than a written path operand.
     * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
     * forbids rewriting such a site as though it spelled the target name.
     */
    bool implicit;
    /** Arena-owned normalized constant member, otherwise NULL. The original
     * whole operand can contain aliases/relative reduction; its text must not
     * be split to recover this name under
     * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02).
     */
    const char *member;
} DeliverySourceDependency;
/** Plain captured same-basename companion metadata. The source identity is
 * exact, paths/ABI borrow the prepared input owner, and absent values explicitly
 * mean captured absence. The compiler binds these only after its production
 * parser establishes a native definition; ordinary/draft companions remain
 * opaque under [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef struct DeliverySourceCompanions {
    struct DeliverySourceCompanions *next;
    const char *identity;
    const char *body;
    const char *header;
    const SourceText *abi;
} DeliverySourceCompanions;
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
