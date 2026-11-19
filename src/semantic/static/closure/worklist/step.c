/** @file
 * @brief Refine all required ordinary representatives and preserve every newly discovered guarded dependency.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) counts new evidence rather than retries or allocation activity.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) shares body work while retaining incoming path checks.
 */
#include "../demands/internal.h"
#include "../../publication/progress.h"

/** Every component counts monotone semantic facts or required proof receipts;
 * arena usage, term interning, elapsed time and retry count are excluded.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits refinement only from these exact new facts and proof receipts.
 */
typedef struct {
    StaticPublicationProgress publication;
    size_t entries, paths, candidates, selections, values, constant_proofs;
} StaticClosureProgress;

/** Qualified transfers can acquire a first namespace proof without new equations;
 * include that receipt as progress, independently of declared constant values.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits refinement only from these exact new facts and proof receipts.
 */
static StaticClosureProgress progress(const StaticClosurePlan *plan) {
    const StaticConstantPlan *constants = plan->connections->constants;
    const StaticRegistryPlan *registry = constants->publications->results->registry;
    StaticClosureProgress value = {.publication = static_publication_progress(constants->publications),
        .entries = plan->count, .paths = registry->guards->path_count,
        .candidates = registry->candidate_count, .selections = constants->selection_count, .values = constants->value_count};
    for (const StaticTypedConstant *item = plan->connections->first_constant; item; item = item->next) {
        value.constant_proofs += item->equations_ready;
    }
    return value;
}

/** Compare exact named counters; never struct padding, a digest or a pass budget.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits refinement only from these exact new facts and proof receipts.
 */
static bool same_progress(StaticClosureProgress a, StaticClosureProgress b) {
    return static_publication_same_progress(a.publication, b.publication) &&
        a.entries == b.entries && a.paths == b.paths && a.candidates == b.candidates &&
        a.selections == b.selections && a.values == b.values && a.constant_proofs == b.constant_proofs;
}

/** A representative's actual annotation applications retain creation edges;
 * requiring its known fields/call targets only adds validation dependencies.
 * Pending ordinary literal capability work belongs to T185, not Type inference.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) separates inference from capability checks.
 */
static bool bodies(StaticClosurePlan *plan, StaticClosureStep *report) {
    for (StaticClosureEntry *entry = plan->first; entry; entry = entry->next) {
        if (entry->representative != entry) { continue; }
        if (!static_closure_ordinary_pass(plan->connections, entry->context, &entry->report)) {
            (void)static_closure_dependencies(plan); return false;
        }
        report->changed |= entry->report.changed;
        report->types_closed &= entry->report.fields_closed;
        bool ready;
        if (!static_closure_fields(plan, entry, &ready)) { return false; }
        report->types_closed &= ready;
        if (!static_closure_operations(plan, entry, &ready)) { return false; }
        report->types_closed &= ready;
    }
    return true;
}

/** New actual contexts are processed in a later finite pass. Static graph
 * evaluation stays in the existing publication component; all guard keys and
 * original registry uses are refreshed before reporting Type closure.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks cached descendants after refinement.
 */
bool static_closure_step(StaticClosurePlan *plan, StaticClosureStep *output) {
    if (!static_closure_dependencies(plan)) { return false; }
    if (!output) {
        return static_closure_fail(plan, STATIC_CLOSURE_INVALID, (SourceSpan){0}, "Missing whole-worklist refinement report");
    }
    plan->closed = false;
    StaticClosureProgress before = progress(plan);
    StaticClosureStep report = {.types_closed = true};
    bool collected;
    if (!static_closure_collect(plan, &collected)) { return false; }
    report.changed |= collected;
    size_t contexts = plan->connections->constants->publications->results->context_count;
    if (!bodies(plan, &report)) { return false; }
    bool ready;
    if (!static_closure_constants(plan, &ready)) { return false; }
    report.types_closed &= ready;
    StaticResultPlan *results = plan->connections->constants->publications->results;
    StaticRegistryKeyState keys;
    if (!static_registry_refresh_keys(results->registry, &keys)) {
        (void)static_closure_dependencies(plan); return false;
    }
    report.types_closed &= keys == STATIC_REGISTRY_KEY_CLOSED && contexts == results->context_count;
    report.changed |= !same_progress(before, progress(plan));
    *output = report;
    return true;
}
