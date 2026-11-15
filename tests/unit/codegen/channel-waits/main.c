/** @file
 * @brief Execute cooperative channel task cancellation and isolated actor failure.
 */
#include "support/install.c"

/** Backpressure yields to ready work; native readiness/cancellation drives only the appropriate actor. */
static void it_runs_cooperative_channel_waits_cancellation_and_isolated_actor_failure(void) {
    install_wait_fixture();
    char source[16384], consumer[16384];
    read_fixture("project/main", source, sizeof(source));
    read_fixture("support/consumer.c", consumer, sizeof(consumer));
    host_launcher = consumer;
    native_entry_override = wait_actor_entry;
    native_hook_override = channel_port_hook;
    native_fixture_completion = complete_wait_native;
    execute_project(source,
        "full\nready\ndecoded\nproducer complete\ndecoded\n"
        "full\nready\ndecoded\n"
        "full\nready\nproducer failed\n"
        "full\nready\ncancelled\ndecoded\n"
        "full\nready\ndecoded\nproducer complete\ndecoded\n", true);
    expect_diagnostic("Channel actor failed");
    native_fixture_completion = NULL;
    native_hook_override = NULL;
    native_entry_override = NULL;
    host_launcher = NULL;
}

/** Run actual task/library/codec/native cancellation and context failure composition. */
void run(void) { it_runs_cooperative_channel_waits_cancellation_and_isolated_actor_failure(); }
/** Return only after generated release ownership checks and independent expected output pass. */
int main(void) { run(); return EXIT_SUCCESS; }
