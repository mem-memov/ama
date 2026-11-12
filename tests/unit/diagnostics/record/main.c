/**
 * @file
 * @brief Check structured diagnostic source/related records before rendering.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"
#include "diagnostics/adapters/adapters.h"

/**
 * Assert that the component keeps primary cause expected help and multiple related spans
 * separate. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_keeps_primary_cause_expected_help_and_multiple_related_spans_separate(void) {
    setup_test();
    DiagnosticRelated notes[] = {
        {{{source, 3, 9}, NULL}, "declaration", "商品"},
        {{{source, 10, 20}, NULL}, "required type", ".shop.item"}
    };
    Diagnostic diagnostic = {DIAGNOSTIC_TYPE_MISMATCH, "Incompatible declared type", "商品",
        {{source, 3, 9}, NULL}, "Source and destination types differ", ".shop.item",
        "Use a value with the declared type", notes, 2};
    assert(diagnostic_valid(&diagnostic));
    assert(diagnostic.primary.span.source == source && diagnostic.related_count == 2);
    assert(strcmp(diagnostic_code_name(diagnostic.code), "type-mismatch") == 0);
    assert(strcmp(diagnostic.expected, ".shop.item") == 0);
    destroy_test();
}

/**
 * Assert that the component rejects invalid spans mismatched paths and missing related arrays.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_invalid_spans_mismatched_paths_and_missing_related_arrays(void) {
    setup_test();
    Diagnostic diagnostic = {0};
    diagnostic.message = "Invalid input";
    assert(diagnostic_valid(&diagnostic));
    diagnostic.primary.span = (SourceSpan){source, 0, source_length(source) + 1};
    assert(!diagnostic_valid(&diagnostic));
    diagnostic.primary.span.end = 0;
    diagnostic.primary.path = "wrong/file";
    assert(!diagnostic_valid(&diagnostic));
    diagnostic.primary.path = NULL;
    diagnostic.related_count = 1;
    assert(!diagnostic_valid(&diagnostic));
    destroy_test();
}

/**
 * Assert that the component adapts every source failure without allocating messages. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_adapts_every_source_failure_without_allocating_messages(void) {
    setup_test();
    for (int status = MODULE_MISSING; status <= MODULE_INVALID_UTF8; status++) {
        ModuleIssue issue = {(ModuleStatus)status, "商店/item", "other/item", NULL, 0};
        if (status == MODULE_INVALID_UTF8) {
            issue.source = source;
            issue.offset = 3;
        }
        Diagnostic diagnostic;
        DiagnosticRelated note;
        bool converted = diagnostic_from_module(&issue, &note, &diagnostic);
        assert(converted && diagnostic_valid(&diagnostic));
        assert(diagnostic.related == &note && diagnostic.related_count == 1);
        if (status == MODULE_INVALID_UTF8) {
            assert(diagnostic.primary.span.start == 3 && diagnostic.primary.span.end == 4);
            assert(diagnostic.expected != NULL);
        }
        if (status == MODULE_OUT_OF_MEMORY) {
            assert(diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        }
    }
    destroy_test();
}

/**
 * Assert that the component retains both original names for an identity collision. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_retains_both_original_names_for_an_identity_collision(void) {
    setup_test();
    IdentityName previous = {0};
    previous.source_name = ".旧.商品";
    previous.declaration = (SourceSpan){source, 3, 9};
    DiagnosticRelated related;
    Diagnostic diagnostic;
    bool converted = diagnostic_from_identity_collision(&previous, ".新.商品",
        (SourceSpan){source, 10, 20}, &related, &diagnostic);
    assert(converted && diagnostic_valid(&diagnostic));
    assert(strcmp(diagnostic.subject, ".新.商品") == 0);
    assert(strcmp(diagnostic.related[0].name, ".旧.商品") == 0);
    assert(diagnostic.related[0].location.span.source == source);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_keeps_primary_cause_expected_help_and_multiple_related_spans_separate();
    it_rejects_invalid_spans_mismatched_paths_and_missing_related_arrays();
    it_adapts_every_source_failure_without_allocating_messages();
    it_retains_both_original_names_for_an_identity_collision();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
