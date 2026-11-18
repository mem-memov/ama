/**
 * @file
 * @brief Normalize one ordinary operation independently of selected actions and static bindings.
 *
 * [CALL-06](../../../../../spec/language/calls-and-results.md#call-06) preserves
 * independent runtime positions and allowed omissions/modes.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) excludes
 * selected actions from the ordinary argument chain.
 * [TPL-09](../../../../../spec/language/templates.md#tpl-09) selects shared
 * fields for constant reads rather than entry inputs/results.
 */
#include <assert.h>
#include "../normalize.h"
#include "../internal.h"

/** Enforce the structural call-mode contract before associating a field; no
 * target is acquired even on an otherwise valid argument list.
 * Contract: [CALL-06](../../../../../spec/language/calls-and-results.md#call-06).
 */
static bool allowed(BindingPlan *plan, AstCallMode mode, const AstArgument *argument) {
    bool read = mode == AST_RETAINED_READ || mode == AST_CONSTANT_READ;
    if ((mode == AST_ENTRY_CALL && argument->kind == AST_SAVED_OUTPUT)
        || (mode == AST_RESUME_CALL && argument->kind != AST_OUTPUT)
        || (read && (argument->kind != AST_SAVED_OUTPUT || argument->omitted))) {
        return binding_invalid(plan, argument->marker, "Request marker or omission is invalid for this operation mode");
    }
    return true;
}

/** Normalize only ordinary arguments; selected children invoke this API on
 * their own AST_CALL nodes. Raw payloads and storage labels stay untouched.
 * Contract: [CALL-06](../../../../../spec/language/calls-and-results.md#call-06),
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09).
 * [TPL-09](../../../../../spec/language/templates.md#tpl-09) requires the
 * actual shared-constant surface when this is a constant read.
 */
bool binding_normalize_runtime(BindingPlan *plan, const BindingInterface *view,
    const AstNode *operation, BindingApplication **output) {
    if (plan->status != BINDING_OK) { return false; }
    assert(operation != NULL && operation->kind == AST_CALL && output != NULL);
    AstCallMode mode = operation->as.call.mode;
    BindingApplication *application = NULL;
    if (!binding_application_begin(plan, view, operation->span, BINDING_RUNTIME_APPLICATION, &application)) { return false; }
    if (view != NULL && ((mode == AST_CONSTANT_READ) != (view->surface == BINDING_CONSTANT_SURFACE))) {
        return binding_invalid(plan, operation->span, "Actual runtime interface surface differs from this operation");
    }
    application->operation = operation;
    for (const AstArgument *argument = operation->as.call.first; argument != NULL; argument = argument->next) {
        if (!allowed(plan, mode, argument)) { return false; }
        BindingRequest request = {.span = argument->span, .marker = argument->marker,
            .direction = argument->kind == AST_INPUT ? BINDING_RUNTIME_INPUT : BINDING_RUNTIME_OUTPUT,
            .named = argument->label != NULL, .omitted = argument->omitted, .syntax.runtime_binding = argument};
        if (argument->label != NULL) {
            request.label = argument->label->name;
            request.reference = argument->label->reference;
            request.list = argument->label->whole_list;
        }
        if (!binding_request_append(plan, application, request)) { return false; }
    }
    binding_application_publish(plan, application, output);
    return true;
}
