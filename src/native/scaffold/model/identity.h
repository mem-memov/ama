/** @file
 * @brief Register category/role identities on complete concrete semantic owner keys.
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) separates generated name categories.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) keeps unequal captures distinct.
 */
#ifndef AMA_SCAFFOLD_MODEL_IDENTITY_H
#define AMA_SCAFFOLD_MODEL_IDENTITY_H
#include "native/scaffold/scaffold.h"

/** Register a complete owner/category key with one optional role/member component.
 * Borrow the checked semantic type and member bytes for this synchronous call;
 * the scaffold registry owns returned encoding/name bytes. Temporary descriptors
 * expire before return. Preserve the first error and never manufacture suffixes.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) compares complete encodings before sharing names.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial preparation through refusal.
 */
const IdentityName *scaffold_identity(NativeScaffold *, const SemanticType *, IdentityCategory, const char *, SourceSpan);
#endif
