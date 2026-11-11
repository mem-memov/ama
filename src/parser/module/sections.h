#ifndef AMA_PARSER_SECTIONS_H
#define AMA_PARSER_SECTIONS_H

#include "lexer/layout/layout.h"

typedef enum { MODULE_DRAFT, MODULE_CONSTANTS, MODULE_AMA, MODULE_NATIVE } ParsedModuleKind;
typedef struct {
    const LayoutLine *marker;
    const LayoutLine *first;
    const LayoutLine *end;
} ParsedSection;
typedef struct {
    ParsedModuleKind kind;
    ParsedSection constants;
    ParsedSection parameters;
    ParsedSection results;
    ParsedSection implementation;
} ModuleSections;

/* Recognize genuine activation markers before validating active whitespace.
 * Output is published only on success; section views borrow layout. This stage
 * checks section structure only; declarations/body are parsed by their stages. */
bool parser_sections(SourceLayout *layout, ModuleSections *output, Diagnostic *error);

#endif
