/**
 * @file
 * @brief Represent borrowed section intervals and implementation activation kind.
 *
 * [SYN-03](../../../spec/language/source-and-names.md#syn-03)
 * Section structure is separate from declaration/body parsing; strict inactive layout remains T022/T023.
 */
#ifndef AMA_PARSER_SECTIONS_H
#define AMA_PARSER_SECTIONS_H

#include "lexer/layout/layout.h"
#include "parser/error/error.h"

/** Retained activation kind, independent of semantic entry eligibility. */
typedef enum { MODULE_DRAFT, MODULE_CONSTANTS, MODULE_AMA, MODULE_NATIVE } ParsedModuleKind;
/** Borrowed half-open physical-node interval owned by the surrounding layout. */
typedef struct {
    const LayoutLine *marker;
    const LayoutLine *first;
    const LayoutLine *end;
} ParsedSection;
/** Complete local section selection, published only after monotone order validation. */
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
/** Publish borrowed half-open section intervals only on success. Recognize retained activation/order without parsing declarations or bodies; T022/T023 supply full current inactive-layout validation.
 */
bool parser_sections(SourceLayout *layout, ModuleSections *output, ParseError *error);

#endif
