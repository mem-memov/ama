/* Call bodies directly to test state selection independently of Ama call resets.
    Generated C names are internal; this is not a public embedding API.
*/
#define main ama_generated_entry
#include "program.c"
#undef main

int main(void) {
    ama_state_43d7649607aeb385 first = {0};
    ama_state_43d7649607aeb385 second = {0};
    ama_call_43d7649607aeb385(&first);
    ama_call_43d7649607aeb385(&first);
    ama_call_43d7649607aeb385(&second);
    ama_call_43d7649607aeb385(&first);
    return 0;
}
