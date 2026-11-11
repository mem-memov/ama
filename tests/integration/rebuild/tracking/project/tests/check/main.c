#include "../common.c"

static void it_reads_the_production_value(void) {
    int value = selected_value();
    assert(value == FIXTURE_VALUE);
}

void run(void) {
    it_reads_the_production_value();
}

int main(void) {
    run();
    return 0;
}
