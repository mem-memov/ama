/** @file
 * @brief Independently verify bounded result-call frames, retained ancestors and refusal atomicity.
 */
#include "common.c"
#include "support/protection.c"
#include "support/refusal.c"

/** Run each explicit frame/authority component contract. */
void run(void) {
    it_keeps_each_ancestor_acquired_and_restores_it_without_allocating();
    it_refuses_before_parking_and_checks_frame_storage();
}

/** Return success after every component assertion and counted cleanup completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
