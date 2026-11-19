/**
 * @file
 * @brief Validate and append requests against frozen independent interface sequences.
 *
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires named
 * static residuals and forbids input-after-output.
 * [CALL-06](../../../../../spec/language/calls-and-results.md#call-06) makes
 * explicit skips consume positions. [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * preserves runtime label storage forms. [COMP-04](../../../../../spec/implementation/compiler.md#comp-04)
 * retains prior binding and actual declaration sites without reporting allocation.
 */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Source names are complete bounded byte views, never NUL-terminated guesses.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool named(BindingName name, SourceSpan label) {
    size_t length = label.end - label.start;
    return name.length == length && memcmp(name.bytes, source_bytes(label.source) + label.start, length) == 0;
}

/** Resolve a whole name, or an ordinary runtime ordinal, in the unchanged snapshot. No successful earlier
 * binding removes a slot or renumbers subsequent positional requests.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static const BindingSlot *select(const BindingSequence *sequence, const BindingRequest *request, size_t position) {
    for (const BindingSlot *slot = sequence->first; slot != NULL; slot = slot->next) {
        if (request->named ? named(slot->name, request->label) : slot->sequence_ordinal == position) { return slot; }
    }
    return NULL;
}

/** Duplicate names are already contradictory within one direction even before
 * actual origin is known. Known source fields add their actual declaration
 * context; intrinsic fields never receive fabricated declaration spans.
 * Input/output labels remain separate namespaces here.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12),
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04).
 */
static bool unique(BindingPlan *plan, const BindingApplication *application, const BindingRequest *request) {
    if (!request->named) { return true; }
    for (const BindingRequest *prior = application->first; prior != NULL; prior = prior->next) {
        if (prior->direction != request->direction) { continue; }
        BindingName previous = {source_bytes(prior->label.source) + prior->label.start,
            prior->label.end - prior->label.start, prior->label};
        if (!named(previous, request->label)) { continue; }
        binding_invalid(plan, request->label, "The interface name is bound more than once");
        plan->related[0] = (DiagnosticRelated){.location = {.span = prior->span}, .role = "earlier binding"};
        plan->error.related = plan->related;
        plan->error.related_count = 1;
        if (prior->slot != NULL && source_span_valid(prior->slot->name.site)) {
            const StaticModule *origin = application->interface->origin;
            plan->related[1] = (DiagnosticRelated){.location = {.span = prior->slot->name.site},
                .role = "actual interface declaration", .name = origin->definition->source->logical_path};
            plan->error.related_count = 2;
        }
        return false;
    }
    return true;
}

/** All checks precede allocation/link/counter mutation. Unknown-origin records
 * retain a null slot; this never certifies their eventual arity/foreign label.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12),
 * [CALL-06](../../../../../spec/language/calls-and-results.md#call-06).
 */
bool binding_request_append(BindingPlan *plan, BindingApplication *application, BindingRequest request) {
    if (plan->status != BINDING_OK) { return false; }
    assert((size_t)request.direction < BINDING_SEQUENCE_COUNT);
    assert((application->kind == BINDING_STATIC_APPLICATION) == (request.direction < BINDING_RUNTIME_INPUT));
    assert(!request.named || source_span_valid(request.label));
    if (application->kind == BINDING_STATIC_APPLICATION && (!request.named || request.omitted)) {
        return binding_invalid(plan, request.span, "Static requests require a name and an operand");
    }
    bool output = request.direction == BINDING_STATIC_OUTPUT || request.direction == BINDING_RUNTIME_OUTPUT;
    BindingMode mode = request.named ? BINDING_MODE_NAMED : BINDING_MODE_POSITIONAL;
    if (application->mode != BINDING_MODE_EMPTY && application->mode != mode) {
        return binding_invalid(plan, request.span, "Named and positional requests cannot mix at one level");
    }
    if (!output && application->output_count != 0) {
        return binding_invalid(plan, request.marker, "Input requests must precede output requests");
    }
    if (request.named && request.omitted) { return binding_invalid(plan, request.span, "A named request cannot omit its operand"); }
    if (!unique(plan, application, &request)) { return false; }
    size_t *counter = output ? &application->output_count : &application->input_count;
    if (*counter == SIZE_MAX) { return binding_memory_failure(plan); }
    request.source_ordinal = *counter;
    request.slot = NULL;
    if (application->interface != NULL) {
        request.slot = select(&application->interface->sequences[request.direction], &request, *counter);
        if (request.slot == NULL) {
            return binding_invalid(plan, request.named ? request.label : request.marker,
                request.named ? "Unknown name in the actual interface sequence" : "Positional request exceeds the actual interface sequence");
        }
        if (request.named && request.direction >= BINDING_RUNTIME_INPUT
            && (request.reference != request.slot->reference || request.list != request.slot->list)) {
            return binding_invalid(plan, request.label, "Runtime binding label storage form differs from its declaration");
        }
    }
    BindingRequest *record = binding_allocate(plan, sizeof(*record));
    if (record == NULL) { return false; }
    *record = request;
    record->next = NULL;
    if (application->last == NULL) { application->first = record; }
    else { application->last->next = record; }
    application->last = record;
    application->mode = mode;
    (*counter)++;
    return true;
}
