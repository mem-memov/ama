#ifndef TEST_ECHO_H
#define TEST_ECHO_H
#include "runtime/value.h"
#include <stdbool.h>
typedef struct {
    size_t executions;
    bool fail;
} EchoState;
typedef EchoState ama_state_e957b3907961d9c0;
extern const AmaType ama_type_e957b3907961d9c0;
#define echo_descriptor ama_type_e957b3907961d9c0
#endif
