/** @file
 * @brief Check all normalization allocation refusals, prior publications and bounded counter invariants.
 */
#include "../request-common.c"
#include "../runtime-common.c"
#include <stdint.h>

/** Both independent request kinds have two inputs/outputs and immutable source
 * owners, enabling exactly the same failure partitions without evaluating them.
 */
static const char *failure_body = "/test/fixture/v1.0.0/src/other\n .a < .type\n .b < .type\n"
    " .x > .received\n .y > .ignored\n";

/** Source kinds remain separate even when testing the same resource partition. */
static void setup_failure(bool runtime) {
    if (runtime) {
        setup_runtime_requests("worker\n/test/fixture/v1.0.0/src/other\nworker !\n <\n <\n >\n > received list[]\n");
    } else {
        setup_request_definitions(failure_body, "@...\n.x\n.a\n.y\n.b\n");
    }
}

/** The actual runtime instance has both static inputs fixed; constructors used
 * by static requests instead expose the original residual input sequence.
 */
static BindingInterface *failure_interface(bool runtime) {
    if (!runtime) {
        BindingInterface *view = NULL;
        assert(binding_source_interface(&binding_plan, binding_module(project_find(&project, "/test/fixture/v1.0.0/src/other")),
            BINDING_MODULE_SURFACE, NULL, 0, &view));
        return view;
    }
    const StaticModule *module = binding_module(project_find(&project, "/test/fixture/v1.0.0/src/other"));
    const StaticSymbol *captures[] = {
        static_symbol_find(module, static_name_site(module->definition, ".a")),
        static_symbol_find(module, static_name_site(module->definition, ".b"))};
    BindingInterface *view = NULL;
    assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, captures, 2, &view));
    return view;
}

/** Locate this fixture's ordinary call without following alias/selected links. */
static const AstNode *failure_call(void) {
    for (const AstNode *node = project.entry->syntax.body.first; node != NULL; node = node->next) {
        if (node->kind == AST_CALL && node->as.call.target != NULL && layout_text_is(node->as.call.target->name, "worker")) { return node; }
    }
    assert(false);
    return NULL;
}

/** Exercise each public normalizer with the same independently prepared facts. */
static bool normalize(bool runtime, const BindingInterface *view, BindingApplication **output) {
    return runtime ? binding_normalize_runtime(&binding_plan, view, failure_call(), output)
        : binding_normalize_static(&binding_plan, view, request_head("/test/fixture/v1.0.0/src/other")->expression, output);
}

/** Every observed allocation is refused once for each kind and known/pending
 * origin. A second failing application never modifies a prior published chain,
 * its output pointer or the immutable interface/source metadata.
 */
static void it_unwinds_every_normalization_allocation_failure_without_mutating_prior_results(void) {
    for (size_t runtime = 0; runtime < 2; runtime++) {
        for (size_t known = 0; known < 2; known++) {
            size_t total = 0;
            for (size_t failure = 0; failure <= total; failure++) {
                setup_failure(runtime != 0);
                BindingInterface *view = known != 0 ? failure_interface(runtime != 0) : NULL;
                BindingApplication *previous = NULL;
                assert(normalize(runtime != 0, view, &previous));
                const BindingRequest *first = previous->first;
                BindingApplication *output = previous;
                size_t before = allocation_count;
                if (failure != 0) { failure_at = before + failure; }
                bool success = normalize(runtime != 0, view, &output);
                if (failure == 0) { assert(success && output != previous); total = allocation_count - before; assert(total != 0); }
                else {
                    assert(!success && output == previous && previous == binding_plan.first_application && previous == binding_plan.last_application);
                    assert(previous->next == NULL && previous->first == first && previous->input_count == 2 && previous->output_count == 2);
                    assert(previous->pending == (known == 0));
                    assert(binding_plan.status == BINDING_OUT_OF_MEMORY && diagnostic_valid(&binding_plan.error));
                    size_t after = allocation_count;
                    assert(!normalize(runtime != 0, view, &output) && output == previous && allocation_count == after);
                    assert(!binding_invalid(&binding_plan, previous->first->span, "later error") && binding_plan.status == BINDING_OUT_OF_MEMORY);
                }
                destroy_binding();
            }
        }
    }
}

/** Inject the numeric boundary directly, not an impossible huge source. Each
 * direction refuses before linking, counting or allocating another request.
 */
static void it_checks_every_direction_counter_limit_without_publishing_or_allocating(void) {
    for (size_t direction = 0; direction < BINDING_SEQUENCE_COUNT; direction++) {
        bool runtime = direction >= BINDING_RUNTIME_INPUT, output = direction % 2 != 0;
        setup_failure(runtime);
        BindingApplication application = {.kind = runtime ? BINDING_RUNTIME_APPLICATION : BINDING_STATIC_APPLICATION,
            .pending = true, .input_count = output ? 0 : SIZE_MAX, .output_count = output ? SIZE_MAX : 0};
        BindingRequest request = {.direction = (BindingDirection)direction};
        if (runtime) {
            const AstArgument *raw = failure_call()->as.call.first;
            if (output) { raw = raw->next->next; }
            request.span = raw->span;
            request.marker = raw->marker;
            request.omitted = raw->omitted;
            request.syntax.runtime_binding = raw;
        } else {
            const AstStaticBinding *raw = request_head("/test/fixture/v1.0.0/src/other")->expression->first;
            if (output) { raw = raw->next->next; }
            request.span = raw->span;
            request.marker = raw->marker;
            request.omitted = false;
            request.syntax.static_binding = raw;
            request.named = true;
            request.label = raw->label;
        }
        size_t before = allocation_count;
        assert(!binding_request_append(&binding_plan, &application, request));
        assert(binding_plan.status == BINDING_OUT_OF_MEMORY && allocation_count == before);
        assert(application.first == NULL && application.last == NULL && application.mode == BINDING_MODE_EMPTY);
        assert((output ? application.output_count : application.input_count) == SIZE_MAX);
        assert(binding_plan.first_application == NULL && diagnostic_valid(&binding_plan.error));
        destroy_binding();
    }
}

/** Static requests cannot enter the ordinary positional/skip branches, even
 * when a compiler component supplies a manually prepared malformed request.
 */
static void it_refuses_unnamed_or_omitted_static_requests_before_mutation(void) {
    for (size_t omitted = 0; omitted < 2; omitted++) {
        setup_failure(false);
        const AstStaticBinding *raw = request_head("/test/fixture/v1.0.0/src/other")->expression->first;
        BindingApplication application = {.kind = BINDING_STATIC_APPLICATION, .pending = true};
        BindingRequest request = {.direction = BINDING_STATIC_INPUT, .span = raw->span,
            .marker = raw->marker, .label = raw->label, .named = omitted != 0,
            .omitted = omitted != 0, .syntax.static_binding = raw};
        size_t before = allocation_count;
        assert(!binding_request_append(&binding_plan, &application, request));
        assert(binding_plan.status == BINDING_INVALID && allocation_count == before);
        assert(application.first == NULL && application.last == NULL);
        assert(application.input_count == 0 && application.output_count == 0 && application.mode == BINDING_MODE_EMPTY);
        destroy_binding();
    }
}

/** Identical shape/origin data copied outside this owner is not a published
 * snapshot. Refusal precedes allocation, field access or application publication.
 */
static void it_rejects_unowned_snapshot_copies_without_using_shape_as_identity(void) {
    for (size_t runtime = 0; runtime < 2; runtime++) {
        setup_failure(runtime != 0);
        BindingInterface *view = failure_interface(runtime != 0);
        BindingInterface foreign = *view;
        BindingApplication *application = NULL;
        size_t before = allocation_count;
        assert(!normalize(runtime != 0, &foreign, &application));
        assert(application == NULL && binding_plan.first_application == NULL && binding_plan.status == BINDING_INVALID);
        assert(allocation_count == before && diagnostic_valid(&binding_plan.error));
        assert(binding_plan.first_interface == view && binding_plan.last_interface == view);
        destroy_binding();
    }
}

/** Execute each transactional/counter/identity boundary explicitly. */
static void run(void) {
    it_unwinds_every_normalization_allocation_failure_without_mutating_prior_results();
    it_checks_every_direction_counter_limit_without_publishing_or_allocating();
    it_refuses_unnamed_or_omitted_static_requests_before_mutation();
    it_rejects_unowned_snapshot_copies_without_using_shape_as_identity();
}

/** Return success after every assertion and counted teardown. */
int main(void) { run(); return 0; }
