#include "driver/driver.h"

int main(int argument_count, char **arguments) {
    return driver_run(argument_count, arguments, stdout, stderr);
}
