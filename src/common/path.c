#include "common/path.h"
#include "common/limits.h"

bool normalize_path(const char* input_path, char* normalized_path, Error* error) {
    int length = 0;
    if (*input_path == '.') input_path++;
    while (*input_path) {
        if (length >= MAX_MODULE_PATH_LENGTH - 1)
            return error_set(error, ERROR_LIMIT, 0, "normalized path exceeds %d bytes",
                MAX_MODULE_PATH_LENGTH - 1);
        normalized_path[length++] = *input_path == '.' ? '/' : *input_path;
        input_path++;
    }
    normalized_path[length] = '\0';
    if (!length)
        return error_set(error, ERROR_INVALID, 0, "empty path after normalization");
    return true;
}
