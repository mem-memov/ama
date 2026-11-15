/** @file
 * @brief Release a controlled native producer at the exact generated wait boundary without timing guesses.
 * [ASYNC-07](../../../../../spec/language/async-and-parallelism.md#async-07)
 * requires a wake arriving after the final empty check to remain observable.
 * This test-only hook contains native scalar coordination and no Ama reference.
 */
#ifndef AMA_TEST_SCHEDULER_WAIT_HOOK_H
#define AMA_TEST_SCHEDULER_WAIT_HOOK_H
#include <stdbool.h>
#include <stdatomic.h>
#include <semaphore.h>
/** Native-only test gate shared by the generated assembly's provider and wait wrapper. */
static _Atomic bool ama_test_wait_entered;
/** Open the producer gate before invoking the actual potentially blocking POSIX wait. */
static inline int ama_test_scheduler_wait(sem_t *wake) {
    atomic_store(&ama_test_wait_entered, true);
    return sem_wait(wake);
}
#define AMA_TEST_WAIT_GATE
#define sem_wait ama_test_scheduler_wait
#endif
