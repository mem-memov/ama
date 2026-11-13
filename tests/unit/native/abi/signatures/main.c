/**
 * @file
 * @brief Require successful exact C signatures and explicit rejection of every incompatible boundary.
 */
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "native/toolchain/profile.h"

/** Compile one maintained candidate and require its exact success or mismatch diagnostic.
 * The controlling generic expression checks type only; no native entry address escapes.
 */
static void check_signature(unsigned selected, bool accepted) {
    char selection[sizeof("-DAMA_ABI_CASE=0")];
    int length = snprintf(selection, sizeof(selection), "-DAMA_ABI_CASE=%u", selected);
    assert(length > 0 && (size_t)length < sizeof(selection));
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (freopen("generated/signature.log", "wb", stderr) == NULL) { _exit(126); }
        char *arguments[] = {TEST_REAL_COMPILER, AMA_TOOLCHAIN_REQUIRED_FLAGS,
            "-I" TEST_ROOT "/src", selection, "-fsyntax-only", "project/probe.c", NULL};
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    assert((WEXITSTATUS(status) == 0) == accepted);
    if (!accepted) {
        FILE *errors = fopen("generated/signature.log", "rb");
        assert(errors != NULL);
        char text[8192];
        size_t count = fread(text, 1, sizeof(text) - 1, errors);
        assert(!ferror(errors) && count < sizeof(text) - 1 && fclose(errors) == 0);
        text[count] = '\0';
        assert(strstr(text, "AMA_EXPECTED_SIGNATURE_MISMATCH") != NULL);
    }
}

/** The positive candidate must compile before checking intentional failures. */
static void it_accepts_an_exact_native_constructor_signature(void) {
    check_signature(0, true);
}

/** Reject all six protocol mismatches plus entry and full-size bridge mismatches. */
static void it_rejects_wrong_protocol_entry_and_host_size_signatures(void) {
    for (unsigned selected = 1; selected <= 8; selected++) { check_signature(selected, false); }
}

/** Run explicit positive and negative checked-C interface cases. */
void run(void) {
    it_accepts_an_exact_native_constructor_signature();
    it_rejects_wrong_protocol_entry_and_host_size_signatures();
}

/** Return success only when every configured compiler outcome matches the interface contract. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
