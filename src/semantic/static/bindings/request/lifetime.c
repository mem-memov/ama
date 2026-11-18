/**
 * @file
 * @brief Prepare and publish normalized applications without exposing partial request chains.
 *
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed
 * construction. [TPL-12](../../../../../spec/language/static-results.md#tpl-12)
 * keeps an absent actual origin pending rather than selecting an expected layout.
 */
#include <assert.h>
#include "../internal.h"

/** Validate exact snapshot ownership before borrowing it. A null snapshot is an
 * explicit missing fact; no search by names, signature or field shape occurs.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_application_begin(BindingPlan *plan, const BindingInterface *view, SourceSpan site,
    BindingApplicationKind kind, BindingApplication **output) {
    if (plan->status != BINDING_OK) { return false; }
    assert(output != NULL);
    if (view != NULL) {
        const BindingInterface *owned = plan->first_interface;
        while (owned != NULL && owned != view) { owned = owned->next; }
        if (owned == NULL) { return binding_invalid(plan, site, "Actual interface snapshot belongs to another binding owner"); }
    }
    BindingApplication *application = binding_allocate(plan, sizeof(*application));
    if (application == NULL) { return false; }
    application->kind = kind;
    application->interface = view;
    application->pending = view == NULL;
    *output = application;
    return true;
}

/** Only complete local chains reach the owner publication list. Older pending
 * results stay immutable; consumers select a later resolution explicitly.
 * Contract: [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_application_publish(BindingPlan *plan, BindingApplication *application, BindingApplication **output) {
    if (plan->last_application == NULL) { plan->first_application = application; }
    else { plan->last_application->next = application; }
    plan->last_application = application;
    *output = application;
}
