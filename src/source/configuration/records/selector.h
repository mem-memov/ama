/**
 * @file
 * @brief Share selector spelling admission between typed INI parsing and compilation.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) separates canonical
 * pattern spelling from alias resolution, identity matching and source loading.
 */
#ifndef AMA_SOURCE_CONFIGURATION_RECORDS_SELECTOR_H
#define AMA_SOURCE_CONFIGURATION_RECORDS_SELECTOR_H
#include "source/text/source.h"
/** Validate bounded ordinary/package-alias pattern spelling without lookup or allocation.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) forbids whole dot/parent
 * components and never interprets masks as ordinary source paths.
 */
bool configuration_selector(SourceSpan text);
#endif
