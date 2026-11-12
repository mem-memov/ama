/**
 * @file
 * @brief Exercise source mapping, canonical resolution and real Doxygen output.
 *
 * Each scenario runs in an isolated child; expected observations are asserted
 * here. Python fixtures call the real tools without a second test framework.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** Run one fixture scenario and require its independent contract checks to pass.
 * The child borrows static argv bytes until exec; failure is preserved by the
 * ordinary leaf harness. No shell interpolation or live compiler state is used.
 */
static void require_scenario(const char *name) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execlp("python3", "python3", "project/scenarios.py", TEST_ROOT, name, (char *) NULL);
        _exit(127);
    }
    int status = 0;
    pid_t completed = waitpid(child, &status, 0);
    assert(completed == child);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Prove that code examples and literals never become prose links. */
static void it_preserves_code_literals_and_maps_nested_markdown(void) {
    require_scenario("source");
}

/** Reject stale paths, mismatched rule IDs, duplicate definitions and collisions. */
static void it_rejects_invalid_canonical_links(void) {
    require_scenario("registry");
}

/** Render nested/moved source with real Doxygen and detect tampered destinations. */
static void it_checks_actual_rendered_destinations_and_source_stability(void) {
    require_scenario("rendered");
}

/** Reject new/stale coverage registrations and require independent header builds. */
static void it_checks_current_coverage_and_exposes_unclosed_debt(void) {
    require_scenario("coverage");
}

/** Run all independent documentation invariants through the shared leaf harness. */
void run(void) {
    it_preserves_code_literals_and_maps_nested_markdown();
    it_rejects_invalid_canonical_links();
    it_checks_actual_rendered_destinations_and_source_stability();
    it_checks_current_coverage_and_exposes_unclosed_debt();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
