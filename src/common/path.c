#include "common/path.h"
#include "common/limits.h"

bool normalize_path(const char* in, char* out, Error* error) {
    int length = 0;
    if (*in == '.') in++;
    while (*in) {
        if (length >= MAX_PATH_LEN - 1)
            return error_set(error, ERROR_LIMIT, 0, "normalized path exceeds %d bytes",
                             MAX_PATH_LEN - 1);
        out[length++] = *in == '.' ? '/' : *in;
        in++;
    }
    out[length] = '\0';
    if (!length)
        return error_set(error, ERROR_INVALID, 0, "empty path after normalization");
    return true;
}
