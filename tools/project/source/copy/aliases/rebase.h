/** @file
 * @brief Rebase local aliases through their original resolved targets for rooted-use copy plans.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits parsed target edits with unchanged keys and rooted source occurrences;
 * [PKG-04](../../../../../spec/project/packages.md#pkg-04) supplies original scope.
 */
#ifndef AMA_SOURCE_COPY_ALIAS_REBASE_H
#define AMA_SOURCE_COPY_ALIAS_REBASE_H
#include "../../edits/edits.h"
#include "source/configuration/aliases/aliases.h"
/** Input/scope authority and edit refusal have distinct diagnostics. */
typedef enum { SOURCE_COPY_ALIASES_OK, SOURCE_COPY_ALIASES_INVALID,
    SOURCE_COPY_ALIASES_SCOPE, SOURCE_COPY_ALIASES_EDITS } SourceCopyAliasesStatus;
/** Borrow exact parsed alias and normalized scope entry through caller teardown. */
typedef struct {
    const ConfigurationAlias *alias;
    const ConfigurationAliasEntry *entry;
    SourceEditsStatus edit_status;
} SourceCopyAliasesIssue;
/** Replace every parsed module-INI alias target with its original fully normalized
 * target. Document and scope must be valid common-parser/resolver results for the
 * same original text as edits; scope is its own MODULE table, not a package table.
 * Keep keys/comments/line endings/other bytes unchanged. Equal repeat edits share
 * normal edit rows. No alias import, target loading, discovery or mutation occurs.
 *
 * This is the explicit strategy for a copy plan that rewrites actual module-use
 * occurrences to their required rooted targets separately. It preserves the old
 * alias bindings without borrowing the old package's alias table. The outer plan
 * must still validate destination nonshadowing, final scope and every actual use;
 * this helper alone proves neither copied-definition bindings nor publication.
 *
 * Input owners outlive edits/issue use. Applied edits or missing issue refuse
 * unchanged. Other failures poison application while retaining owned prefixes.
 * O(A squared) exact span lookups plus normal edit insertion/target bytes for A
 * aliases; storage is owned edit rows/replacement bytes only.
 */
SourceCopyAliasesStatus source_copy_rebase_aliases(SourceTextEdits *, const ConfigurationDocument *,
    const ConfigurationAliasScope *, SourceCopyAliasesIssue *);
#endif
