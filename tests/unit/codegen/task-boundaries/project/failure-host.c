/** @file
 * @brief Require terminal context cleanup before failed reaches the native host, then acknowledge callbacks without any Ama state.
 */
#include "project.h"
#include "project.c"
#include <assert.h>
#include <stdio.h>
/** A failed context cannot run another task or resume an old continuation. */
int main(void) {
    AmaProjectHost host = {0};
    AmaHostResult first = ama_project_execute(&host, AMA_ACCESS_ENTRY);
    assert(first.outcome == AMA_HOST_FAILED && first.status.code != AMA_OK);
    AmaHostResult repeated = ama_project_execute(&host, AMA_ACCESS_RESUME);
    assert(repeated.outcome == AMA_HOST_FAILED && repeated.status.code != AMA_OK);
    ama_n_d2428efd14daaa304f9b7880ff26f283437ea6852eb1aa2c4b93e3fb1421d52b_acknowledge();
    AmaStatus disposed = ama_project_dispose(&host);
    assert(disposed.code != AMA_OK && host.disposed && !host.registry.initialized);
    puts("host=failed");
    return 0;
}
