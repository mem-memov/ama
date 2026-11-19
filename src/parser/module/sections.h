/**
 * @file
 * @brief Represent borrowed section intervals and implementation activation kind.
 *
 * [SYN-03](../../../spec/language/source-and-names.md#syn-03)
 * Section structure is separate from declaration/body parsing; shared lexical checks precede draft exclusion.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) gives static modules
 * their own exclusive graph interval, never an ordinary implementation section.
 */
#ifndef AMA_PARSER_SECTIONS_H
#define AMA_PARSER_SECTIONS_H

#include "lexer/layout/layout.h"
#include "parser/error/error.h"

/** Retained activation kind, independent of semantic entry eligibility. Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
typedef enum { MODULE_DRAFT, MODULE_CONSTANTS, MODULE_AMA, MODULE_NATIVE, MODULE_STATIC } ParsedModuleKind;
/** Borrowed half-open physical-node interval owned by the surrounding layout. Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
typedef struct {
    const LayoutLine *marker;
    const LayoutLine *first;
    const LayoutLine *end;
} ParsedSection;
/** Complete local section selection, published only after monotone order validation. Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
typedef struct {
    ParsedModuleKind kind;
    ParsedSection constants;
    ParsedSection parameters;
    ParsedSection results;
    ParsedSection implementation;
    /** Exclusive compile-time graph, not an executable implementation interval.
     * [TPL-12](../../../spec/language/static-results.md#tpl-12) forbids ordinary sections here.
     */
    ParsedSection graph;
} ModuleSections;

/* Check shared lexical rules, then recognize genuine activation markers.
 * Output is published only on success; section views borrow layout. This stage
 * checks section structure only; declarations/body are parsed by their stages. */
/** Publish borrowed half-open section intervals only on success. Check shared lexical tokens and section order without parsing declarations or bodies.
 * Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
bool parser_sections(SourceLayout *layout, ModuleSections *output, ParseError *error);

/** Check shared lexical whitespace/tokens and publish whether a genuine activation marker exists.
 * Incomplete draft declarations need not parse. Comments retain indentation for
 * later active-region validation. Output changes only on complete success.
 * Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
bool parser_module_classify(const SourceLayout *, bool *, ParseError *);
#endif
