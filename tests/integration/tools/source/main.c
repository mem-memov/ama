/** @file
 * @brief Accept editor/rename command processes with exact Unicode identities, unused importers and unchanged original documents.
 */
#include "../../common.c"

/** JSON semantics are checked independently of the writer; the C harness owns process orchestration and every assertion boundary. */
static void verify(const char *mode) {
    assert(fflush(output) == 0 && fseek(output,0,SEEK_SET) == 0);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (dup2(fileno(output),STDIN_FILENO) < 0) { _exit(127); }
        execlp("python3","python3","verify.py",mode,(char *)NULL);
        _exit(127);
    }
    int status;
    assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Static observations use the actual launcher, explicit entry and selected source rather than a standalone alternate parser. */
static void it_queries_a_real_project_through_exact_command_arguments(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE,"--editor","project",".main","--module",".palette","--offset","3",NULL};
    assert(run_compiler_arguments(arguments) == EXIT_SUCCESS);
    verify("editor");
    char message[1024];
    read_stream(errors,message,sizeof(message));
    assert(message[0] == 0);
    destroy_test();
}
/** A checked declaration rename includes unreachable importers while drafts and C bytes remain opaque. */
static void it_prepares_complete_workspace_changes_without_applying_them(void) {
    for (unsigned noop = 0; noop < 2; noop++) {
        setup_test();
        char *arguments[] = {AMA_EXECUTABLE,"--rename-plan","project",".main","--module",".palette","--offset","3",
            "--name",noop ? "общее имя" : "новое общее имя",NULL};
        assert(run_compiler_arguments(arguments) == EXIT_SUCCESS);
        verify(noop ? "noop" : "rename");
        destroy_test();
    }
}
/** Invalid names cannot produce an apparently usable replacement set or mutate the original workspace. */
static void it_reports_refused_process_requests_without_partial_plans(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE,"--rename-plan","project",".main","--module",".palette","--offset","3","--name","bad  name",NULL};
    assert(run_compiler_arguments(arguments) != EXIT_SUCCESS && ftell(output) == 0);
    char message[2048];
    read_stream(errors,message,sizeof(message));
    assert(strstr(message,"<rename>") != NULL && strstr(message,"syntax") != NULL);
    destroy_test();
}
/** Explicitly run full-project process acceptance, not source-generator fixtures in place of the real command. */
void run(void) {
    it_queries_a_real_project_through_exact_command_arguments();
    it_prepares_complete_workspace_changes_without_applying_them();
    it_reports_refused_process_requests_without_partial_plans();
}
/** Successful leaves clean only generated outputs; maintained source inputs remain untouched. */
int main(void) { run(); return EXIT_SUCCESS; }
