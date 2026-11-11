#define OBSERVATION_FILE "../../generated/left"
#include "../common.c"

static void it_executes_between_setup_and_destruction(void) {
    setup_test();
    record_step("run\n");
    assert(!FORCE_FAILURE);
    destroy_test();
}

void run(void) {
    it_executes_between_setup_and_destruction();
}

int main(void) {
    run();
    return 0;
}
