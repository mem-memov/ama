#include <sys/stat.h>
#include "../common.c"

static int run_built_program(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output),STDOUT_FILENO)<0 || dup2(fileno(errors),STDERR_FILENO)<0) { _exit(126); }
        execl("generated/complete/program","generated/complete/program",(char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process,&status,0);
    assert(waited==process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

static void it_builds_and_runs_a_verified_executable_through_the_real_command(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE,"--emit","project",".main","--out","generated/complete","--sdk",TEST_SDK,"--compile",NULL};
    int result = run_compiler_arguments(arguments);
    assert(result == EXIT_SUCCESS);
    struct stat metadata;
    int found = stat("generated/complete/program",&metadata);
    assert(found == 0 && S_ISREG(metadata.st_mode) && (metadata.st_mode & S_IXUSR)!=0);
    result = run_built_program();
    assert(result == EXIT_SUCCESS);
    char bytes[4096];
    read_stream(errors,bytes,sizeof(bytes));
    assert(bytes[0] == 0);
    destroy_test();
}

static void it_preserves_the_previous_program_when_native_compilation_fails(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE,"--emit","project",".broken","--out","generated/complete","--sdk",TEST_SDK,"--compile",NULL};
    int result = run_compiler_arguments(arguments);
    assert(result == EXIT_FAILURE);
    char bytes[8192];
    read_stream(errors,bytes,sizeof(bytes));
    assert(strstr(bytes,"Native compilation must fail")!=NULL);
    result = run_built_program();
    assert(result == EXIT_SUCCESS);
    destroy_test();
}

static void it_refuses_to_overwrite_an_unowned_program(void) {
    setup_test();
    int created = mkdir("generated/conflict",0700);
    assert(created == 0 || errno == EEXIST);
    FILE *file = fopen("generated/conflict/program","wb");
    assert(file != NULL);
    int written = fputs("Handwritten file\n",file);
    int closed = fclose(file);
    assert(written != EOF && closed == 0);
    char *arguments[] = {AMA_EXECUTABLE,"--emit","project",".main","--out","generated/conflict","--sdk",TEST_SDK,"--compile",NULL};
    int result = run_compiler_arguments(arguments);
    assert(result == EXIT_FAILURE);
    file = fopen("generated/conflict/program","rb");
    assert(file != NULL);
    char bytes[64] = {0};
    size_t count = fread(bytes,1,sizeof(bytes)-1,file);
    closed = fclose(file);
    assert(count == strlen("Handwritten file\n") && closed == 0 && strcmp(bytes,"Handwritten file\n")==0);
    destroy_test();
}

void run(void) {
    it_builds_and_runs_a_verified_executable_through_the_real_command();
    it_preserves_the_previous_program_when_native_compilation_fails();
    it_refuses_to_overwrite_an_unowned_program();
}
int main(void) { run(); return EXIT_SUCCESS; }
