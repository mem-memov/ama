/**
 * @file
 * @brief Distinguish structural parse success, invalid input and allocation failure.
 *
 * [COMP-04](../../spec/implementation/compiler.md#comp-04)
 * Explicit status complements original-source diagnostics without guessed recovery.
 */
#ifndef AMA_PARSER_STATUS_H
#define AMA_PARSER_STATUS_H

/** Structural status separates invalid syntax from memory exhaustion; parent valid flags gate consumption. */
typedef enum { PARSE_OK, PARSE_INVALID, PARSE_OUT_OF_MEMORY } ParseStatus;

#endif
