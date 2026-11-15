/** @file
 * @brief Build real C libraries through the product CLI and preserve owned output on failure.
 */
#include "../common.c"
#include <sys/stat.h>
/** Compile a complete explicit component into one separately owned output directory. */
static int build_library(const char *compiler, const char *destination) {
    char *arguments[]={AMA_EXECUTABLE,"--library-build","project","component.exports",
        "--out",(char *)destination,"--sdk",TEST_SDK,"--cc",(char *)compiler,NULL};
    return run_compiler_arguments(arguments);
}
/** Borrow one bounded generated identity from its exact metadata property. */
static void identity(const char *metadata, char name[80]) {
    const char *start=strstr(metadata,"\"identity\":\""); assert(start!=NULL); start+=strlen("\"identity\":\"");
    const char *end=strchr(start,'"'); assert(end!=NULL && end>start && (size_t)(end-start)<80);
    memcpy(name,start,(size_t)(end-start)); name[end-start]=0;
}
/** A genuine C host uses the public header and static archive without SDK includes. */
static void consumer(const char *metadata) {
    char component[80], exported[80]; identity(metadata,component);
    const char *exports=strstr(metadata,"\"exports\":["); assert(exports!=NULL); identity(exports,exported);
    FILE *source=fopen("generated/consumer.c","wb"); assert(source!=NULL);
    int written=fprintf(source,
        "#include \"output with spaces/public.h\"\n#include <assert.h>\nint main(void) {\n"
        "%s_Context context={0}; %s_Instance instance={0}; uint32_t outcome=0;\n"
        "%s_ApiResult result=%s_context_create(&context); assert(result.code==0);\n"
        "result=%s_create(context,&instance); assert(result.code==0);\n"
        "result=%s_entry(context,instance,&outcome); assert(result.code==0 && outcome==2);\n"
        "result=%s_resume(context,instance,&outcome); assert(result.code==0 && outcome==1);\n"
        "result=%s_context_dispose(&context); assert(result.code==0);\n"
        "assert(%s_code_can_unload()==0); return 0; }\n",
        component,component,component,component,exported,exported,exported,component,component);
    int closed=fclose(source); assert(written>0 && closed==0);
    pid_t child=fork(); assert(child>=0);
    if (child==0) {
        execlp(TEST_C_COMPILER,TEST_C_COMPILER,"-std=c11","-Wall","-Wextra","-Werror","-pedantic-errors",
            "generated/consumer.c","generated/output with spaces/library.a","-o","generated/consumer",(char *)NULL); _exit(127);
    }
    int status; pid_t waited;
    do { waited=waitpid(child,&status,0); } while(waited<0 && errno==EINTR);
    assert(waited==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
    child=fork(); assert(child>=0);
    if(child==0) { execl("generated/consumer","generated/consumer",(char *)NULL); _exit(127); }
    do { waited=waitpid(child,&status,0); } while(waited<0 && errno==EINTR);
    assert(waited==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
}
/** Repeated successful builds preserve metadata; a failed rebuild leaves the usable prior library. */
static void it_publishes_repeatable_libraries_and_preserves_outputs_on_tool_failure(void) {
    char first[8192],second[8192],diagnostic[8192];
    setup_test();
    int result=build_library(TEST_C_COMPILER,"generated/output with spaces");
    read_stream(errors,diagnostic,sizeof(diagnostic));
    if(result!=EXIT_SUCCESS) { fputs(diagnostic,stderr); } assert(result==EXIT_SUCCESS);
    read_stream(output,first,sizeof(first));
    struct stat status;
    assert(stat("generated/output with spaces/library.a",&status)==0 && status.st_size>0);
    assert(stat("generated/output with spaces/library.so",&status)==0 && status.st_size>0);
    assert(stat("generated/output with spaces/launcher.c",&status)!=0 && errno==ENOENT);
    consumer(first); destroy_test();
    setup_test(); result=build_library(TEST_C_COMPILER,"generated/output with spaces");
    read_stream(errors,diagnostic,sizeof(diagnostic));
    if(result!=EXIT_SUCCESS) { fputs(diagnostic,stderr); } assert(result==EXIT_SUCCESS);
    read_stream(output,second,sizeof(second)); assert(strcmp(first,second)==0); destroy_test();
    setup_test(); result=build_library("/bin/false","generated/output with spaces");
    read_stream(errors,diagnostic,sizeof(diagnostic)); read_stream(output,second,sizeof(second));
    assert(result!=EXIT_SUCCESS && second[0]==0 && strstr(diagnostic,"io-failure")!=NULL);
    consumer(first); destroy_test();
    setup_test(); result=build_library("/bin/false","generated/failed");
    assert(result!=EXIT_SUCCESS && stat("generated/failed/library.a",&status)!=0); destroy_test();
}
/** Register actual CLI compilation, typed consumer linkage and publication failure containment. */
void run(void) { it_publishes_repeatable_libraries_and_preserves_outputs_on_tool_failure(); }
/** Return after the independently compiled C host succeeds before and after a failed rebuild. */
int main(void) { run(); return EXIT_SUCCESS; }
