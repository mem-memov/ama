/** @file
 * @brief Expose an original retained receiver's Type and written storage form.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes receiver identities before Type inference.
 */
#include "../../internal.h"

/** Exact syntax identities distinguish a fresh field, an existing indexed cell
 * and same-spelled scoped references. No runtime index executes and no final
 * namespace lookup can replace the lexical receiver decision.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) preserves scoped receiver identity.
 */
bool static_typed_receiver_operand(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const AstNode *statement, const AstAccess *syntax, StaticTypedOperand *output) {
    for (const StaticRuntimeReceiver *receiver = scope->runtime->first_receiver; receiver; receiver = receiver->next) {
        if (receiver->statement != statement || receiver->syntax != syntax) { continue; }
        const StaticTypedField *field;
        if (!static_connection_field(plan, scope, receiver->field, &field)) { return false; }
        *output = (StaticTypedOperand){.type = field->type, .field = receiver->field,
            .reference = syntax->reference, .list = syntax->whole_list};
        return true;
    }
    return static_connection_fail(plan, STATIC_CONNECTION_INVALID, statement->span, "Transfer lacks its original lexical receiver");
}
