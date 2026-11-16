/** @file
 * @brief Exercise clean/reused verified products, changed dependencies and current Unicode diagnostics.
 * Each case owns its source/output trees; observed generated text is compared as opaque bytes.
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/** Run one independent product scenario and require its normal successful exit; no shell interprets paths. */
static void scenario(const char *name) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execlp("python3", "python3", "project/scenarios.py", TEST_ROOT, name, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Repeated emission and a fresh output directory reproduce exact source/metadata bytes and executable behavior. */
static void it_reproduces_clean_and_reused_unicode_products_without_changing_inputs(void) { scenario("repeat"); }
/** Actual transitive Ama/header/C/ABI/settings changes affect the next verified executable and leave other inputs intact. */
static void it_rebuilds_verified_products_after_each_declared_input_change(void) { scenario("changes"); }
/** Rejected configuration/collisions preserve prior products, and changed source positions produce current runtime traces. */
static void it_preserves_products_on_refusal_and_refreshes_original_source_diagnostics(void) { scenario("boundaries"); }
/** Explicitly run independent product behavior partitions. */
void run(void) {
    it_reproduces_clean_and_reused_unicode_products_without_changing_inputs();
    it_rebuilds_verified_products_after_each_declared_input_change();
    it_preserves_products_on_refusal_and_refreshes_original_source_diagnostics();
}
/** Return only after every actual compiler and executable observation passes. */
int main(void) { run(); return EXIT_SUCCESS; }
