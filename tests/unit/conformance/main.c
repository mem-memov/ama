/** @file
 * @brief Exercise actual conformance tooling through independent canonical, schema, provenance and C-interface scenarios.
 * Synthetic repositories are private fixture inputs; canonical project specifications remain read-only.
 */
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** The Python helper invokes real checker operations; the explicit C case owns its process and normal-exit assertion. */
static void scenario(const char *name) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execlp("python3", "python3", "project/scenarios.py", TEST_ROOT, name, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t completed = waitpid(child, &status, 0);
    assert(completed == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Duplicate keys and malformed/open claims cannot masquerade as a complete evidence record. */
static void it_rejects_duplicate_keys_invalid_schema_and_open_claims(void) { scenario("schema"); }
/** Canonical IDs, exact targets and all ordered acceptance criteria remain a complete bijection. */
static void it_matches_exact_canonical_rules_and_acceptance_rows(void) { scenario("records"); }
/** Any changed maintained input, registration or argument invalidates old reviewed authority. */
static void it_rejects_changed_input_registry_and_argument_authority(void) { scenario("provenance"); }
/** Referencing a real registration is necessary but insufficient: actual C case signatures must compile. */
static void it_checks_actual_component_and_composition_case_types(void) { scenario("interfaces"); }
/** The pinned parser ignores quoted/fenced/nested fake matrices and preserves escaped table criteria. */
static void it_parses_canonical_tables_without_inventing_nested_obligations(void) { scenario("canonical"); }
/** The repository's final reviewed evidence must pass the same complete checker and actual C probes. */
static void it_checks_the_current_canonical_evidence_and_real_test_interfaces(void) { scenario("current"); }
/** Keep all failure partitions before the complete current-repository acceptance check. */
void run(void) {
    it_rejects_duplicate_keys_invalid_schema_and_open_claims();
    it_matches_exact_canonical_rules_and_acceptance_rows();
    it_rejects_changed_input_registry_and_argument_authority();
    it_checks_actual_component_and_composition_case_types();
    it_parses_canonical_tables_without_inventing_nested_obligations();
    it_checks_the_current_canonical_evidence_and_real_test_interfaces();
}
/** Return only after every exact evidence/provenance/interface assertion succeeds. */
int main(void) { run(); return EXIT_SUCCESS; }
