#include "common/path.h"
#include "common/limits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void normalize_path(const char* in, char* out) {
    int j = 0;
    int start = (in[0] == '.') ? 1 : 0;

    for (int i = start; in[i] != '\0'; i++) {
        if (j >= MAX_PATH_LEN - 1) {
            fprintf(stderr,
                    "error: normalized path exceeds %d bytes: '%.60s%s'\n",
                    MAX_PATH_LEN - 1, in, (strlen(in) > 60) ? "..." : "");
            exit(1);
        }
        out[j++] = (in[i] == '.') ? '/' : in[i];
    }
    out[j] = '\0';

    if (out[0] == '\0') {
        fprintf(stderr,
                "error: empty path after normalization (input: '%s')\n", in);
        exit(1);
    }
}
