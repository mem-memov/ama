/** @file
 * @brief Declare validated original capture lookup and nominal record assembly.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains original order.
 */
#ifndef AMA_STATIC_NOMINAL_INTERNAL_H
#define AMA_STATIC_NOMINAL_INTERNAL_H
#include "../internal.h"
/** Find a supplied original formal after the bounded array has been validated.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) uses exact formal identity.
 */
const StaticTerm *static_nominal_supplied(const StaticTermCapture *, size_t, const StaticSymbol *);
/** Build canonical ordered captures/residuals in unpublished arena storage.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
bool static_nominal_prepare(StaticTermStore *, StaticTerm *, const StaticTermCapture *, size_t);
#endif
