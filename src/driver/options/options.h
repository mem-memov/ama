#ifndef AMA_DRIVER_OPTIONS_H
#define AMA_DRIVER_OPTIONS_H

#include "compiler/frontend.h"
#include "driver/request/request.h"

typedef struct {
    SourceArena arena;
    DriverRequest request;
    ModuleRoot *roots;
    size_t root_count;
    const char *entry;
    const char *output;
    const char *compiler;
    const char *sdk;
    const char **c_flags;
    size_t c_flag_count;
    bool compile;
    bool create;
    bool overwrite;
    const char *protocols;
    Diagnostic error;
    bool valid;
} DriverOptions;

/* Borrow argument strings until destruction; own arrays. Exact options only,
 * no implicit roots/library names or guessed output exclusions. Parse once. */
void driver_options_init(DriverOptions *options, SourceAllocator allocator);
void driver_options_destroy(DriverOptions *options);
bool driver_options_parse(DriverOptions *options, int count, char *const arguments[]);

#endif
