#ifndef AMA_COMPILER_FRONTEND_INTERNAL_H
#define AMA_COMPILER_FRONTEND_INTERNAL_H

#include "frontend.h"

bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries);
bool compiler_frontend_finish(CompilerFrontend *frontend, SemanticLibraries libraries,
    const SemanticNativeAuthoring *authoring);

#endif
