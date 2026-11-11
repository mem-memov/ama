#include "native/abi/services.h"
#include "native/toolchain/gcc.h"

typedef struct { size_t total; } Payload;
typedef struct { const size_t *value; } Inputs;
typedef struct { size_t *value; } Outputs;
