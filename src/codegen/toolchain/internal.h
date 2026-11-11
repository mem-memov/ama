#ifndef AMA_PROJECT_TOOLCHAIN_INTERNAL_H
#define AMA_PROJECT_TOOLCHAIN_INTERNAL_H
#include "codegen/internal.h"
bool codegen_run_tool(NativeScaffold *, char *const *, const char *, FILE *);
bool codegen_compile_object(NativeScaffold *, const char *, FILE *);
bool codegen_verify_object(NativeScaffold *, const char *, FILE *);
bool codegen_link_object(NativeScaffold *, const char *, FILE *);
#endif
