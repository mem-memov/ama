/** @file
 * @brief Compile an SDK-independent consumer and link exact generated component inputs.
 */
/** Compile the consumer without any SDK include path, proving public-boundary independence. */
static void compile_consumer(void) {
    char *arguments[22]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors"};
    size_t count=8;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined"; arguments[count++]="-fno-omit-frame-pointer";
    arguments[count++]="-fno-pie";
#endif
    arguments[count++]="-Igenerated"; arguments[count++]="-c";
    arguments[count++]="consumer/main.c"; arguments[count++]="-o";
    arguments[count++]="generated/consumer.o"; arguments[count]=NULL;
    bool compiled=library_package_process(arguments,stderr); assert(compiled);
}
/** Link separate public-only host C and parent/child compiled code with matching sanitizer settings. */
static void link_nested(const char *dependency) {
    compile_consumer();
    char *arguments[26]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors"};
    size_t count=8;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined"; arguments[count++]="-fno-omit-frame-pointer";
    arguments[count++]="-fno-pie"; arguments[count++]="-no-pie";
#endif
    arguments[count++]="-I" TEST_SDK; arguments[count++]="-Igenerated";
    arguments[count++]="-Igenerated/parent-output"; arguments[count++]="generated/consumer.o";
    arguments[count++]="generated/parent.c"; arguments[count++]="generated/child-output/library.a";
    if (dependency!=NULL) { arguments[count++]=(char *)dependency; }
    arguments[count++]="-o"; arguments[count++]="generated/consumer"; arguments[count]=NULL;
    bool linked=library_package_process(arguments,stderr); assert(linked);
}
