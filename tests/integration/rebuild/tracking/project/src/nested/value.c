#include "value.h"

#ifndef EXTRA_VALUE
#define EXTRA_VALUE 0
#endif

int selected_value(void) {
    return FIXTURE_VALUE + EXTRA_VALUE;
}
