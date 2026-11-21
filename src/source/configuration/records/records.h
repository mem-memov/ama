/**
 * @file
 * @brief Recognize bounded strict INI records before profile/table validation.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) separates exact headers,
 * assignments and dependency selectors; comments occupy complete lines only.
 */
#ifndef AMA_SOURCE_CONFIGURATION_RECORDS_H
#define AMA_SOURCE_CONFIGURATION_RECORDS_H
#include "source/configuration/lines/lines.h"

/** Syntactic categories; assignment meaning is determined by the containing section.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) assigns record meaning through the explicit containing section.
 */
typedef enum {
    CONFIGURATION_BLANK,
    CONFIGURATION_COMMENT,
    CONFIGURATION_DEPENDENCIES,
    CONFIGURATION_ALIASES,
    CONFIGURATION_REPOSITORY,
    CONFIGURATION_ASSIGNMENT,
    CONFIGURATION_SELECTOR /* Bare candidate; the containing section checks selector syntax. */
} ConfigurationRecordKind;

/** Borrowed original spans: repository key is its prefix, assignment key/value
 * exclude exactly the separator, and a selector value is the complete line.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) permits later equals
 * signs and semicolons in values without escape or comment processing.
 */
typedef struct {
    ConfigurationRecordKind kind;
    SourceSpan span;
    SourceSpan key;
    SourceSpan value;
} ConfigurationRecord;

/** Recognize one span already returned by the strict line scanner.
 * No allocation, lookup, byte mutation or section state. Success publishes the
 * complete record; failure preserves output and fills an original issue span.
 * A bare non-assignment is only a selector candidate, not a validated pattern.
 * Section/key uniqueness, profile choice and field meanings belong to document
 * validation. [PKG-03](../../../../spec/project/packages.md#pkg-03) gives the
 * exact joins and rejects unknown header forms. Time O(line bytes), space O(1).
 */
bool configuration_record_read(SourceSpan line, ConfigurationRecord *output,
    ConfigurationIssue *issue);
#endif
