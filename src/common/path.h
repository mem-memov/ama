#ifndef AMA_COMMON_PATH_H
#define AMA_COMMON_PATH_H
#include "common/diagnostic.h"
bool normalize_path(const char* input_path, char* normalized_path, Error* error);
#endif
