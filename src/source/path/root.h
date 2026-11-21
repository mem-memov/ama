/**
 * @file
 * @brief Share authoritative namespace families without depending on filesystem catalogs.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) separates ordinary
 * and standard roots; [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * uses the same families during catalog selection. [SRC-06](../../../spec/language/source-and-names.md#src-06)
 * preserves that family through relative path reduction.
 */
#ifndef AMA_SOURCE_PATH_ROOT_H
#define AMA_SOURCE_PATH_ROOT_H

/** Namespace family, never inferred from physical directory spelling.
 * Zero remains invalid until explicitly configured. The established numeric
 * values are unchanged. [SRC-01](../../../spec/language/source-and-names.md#src-01)
 * makes PROJECT the ordinary slash namespace, including external dependencies.
 */
typedef enum {
    MODULE_ROOT_PROJECT = 1,
    MODULE_ROOT_STANDARD = 2
} ModuleRootKind;
#endif
