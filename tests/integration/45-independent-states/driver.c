/* Call bodies directly to test state selection independently of Ama call resets.
    Generated C names are internal; this is not a public embedding API.
*/
#define main ama_generated_entry
#include "program.c"
#undef main

int main(void) {
    State_main first = {0};
    State_main second = {0};
    module_main(&first);
    module_main(&first);
    module_main(&second);
    module_main(&first);
    return 0;
}
