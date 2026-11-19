/** @file
 * @brief Share exact source-target selection for ordinary interface preparation.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) separates caller accesses and parent result selection.
 */
#ifndef AMA_STATIC_TYPED_CALL_INTERNAL_H
#define AMA_STATIC_TYPED_CALL_INTERNAL_H
#include "../../internal.h"
/** Resolve a root caller field or a parent's public result field, retaining the
 * original index/suffix form. Unknown parent metadata remains Pending; no names
 * are searched outside the selected namespace and no runtime index executes.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) fixes selected-result ownership.
 */
bool static_typed_target(StaticConnectionPlan *, const StaticTypedScope *,
    const AstNode *, StaticTypedOperand *, StaticResultOutcome *);
/** Prepare a whole-list service's exact intrinsic metadata without requiring
 * its element Type to close. Validate only the service's structural profile;
 * native integer capabilities remain the later capability consumer's work.
 * [LIST-04](../../../../../../spec/language/lists.md#list-04) defines the six fixed service methods.
 */
bool static_typed_list_service(StaticConnectionPlan *, const StaticTypedScope *,
    const AstNode *, const StaticTypedOperand *, const StaticTypedOperation **);
/** Prepare a selected method under its exact retained service parent.
 * [LIST-04](../../../../../../spec/language/lists.md#list-04) forbids method values and non-entry modes.
 */
bool static_typed_list_method(StaticConnectionPlan *, const StaticTypedOperation *,
    const AstNode *, const StaticTypedOperation **);
/** Select a method's ephemeral scalar public result from canonical descriptors,
 * without inventing a stored declaration or retaining runtime result state.
 * [LIST-04](../../../../../../spec/language/lists.md#list-04) fixes element-reference and integer output Types.
 */
bool static_typed_list_result(StaticConnectionPlan *, const StaticTypedOperation *,
    const AstNode *, StaticTypedOperand *);
#endif
