/** @file
 * @brief Separate canonical DAG discovery from checked UTF-8 key measurement/writing.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns discovery scratch.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05) requires exact finite traversal.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) retains original UTF-8 names.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) fixes original formal order.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) forbids ambiguous encodings.
 */
#ifndef AMA_STATIC_REGISTRY_KEY_INTERNAL_H
#define AMA_STATIC_REGISTRY_KEY_INTERNAL_H
#include "key.h"
#include "../../terms/internal.h"

/** One scratch vertex; construction-order term ordinals never enter the key.
 * Active ancestry detects cycles exactly, without a depth or iteration budget.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05) requires finite exact traversal.
 */
typedef struct StaticKeyNode {
    struct StaticKeyNode *next_seen;
    struct StaticKeyNode *next_order;
    struct StaticKeyNode *parent;
    const StaticTerm *value;
    const StaticTermCapture *remaining;
    size_t ordinal;
    bool complete;
} StaticKeyNode;

/** Temporary discovery state borrows immutable normalized source/term metadata.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds scratch ownership.
 */
typedef struct {
    SourceArena scratch;
    const StaticTermStore *terms;
    const StaticTermEnvironment *environment;
    StaticKeyNode *seen;
    StaticKeyNode *first;
    StaticKeyNode *last;
    size_t count;
} StaticKeyPlan;

/** A null destination measures the exact same immutable emission as writing.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) requires full counted data.
 */
typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    StaticKeyStatus status;
} StaticKeyWriter;

/** Borrow the complete applied map; caller has validated environment membership.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains original order.
 */
const StaticTermCapture *static_key_inputs(const StaticTermEnvironment *);
/** Discover exactly the reachable DAG in deterministic postorder, with no C recursion.
 * [ARCH-05](../../../../../spec/implementation/compiler.md#arch-05) gives a finite progress measure.
 */
StaticKeyStatus static_key_collect(StaticKeyPlan *);
/** Find an exact already-discovered child; no source-name comparison chooses identity.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves nominal distinctions.
 */
const StaticKeyNode *static_key_find(const StaticKeyPlan *, const StaticTerm *);
/** Append a counted byte range, checking every addition before copying.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) forbids truncated keys.
 */
void static_key_bytes(StaticKeyWriter *, const char *, size_t);
/** Emit one decimal count followed by a colon; zero is represented explicitly.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) fixes exact boundaries.
 */
void static_key_number(StaticKeyWriter *, size_t);
/** Emit a UTF-8 field as byte length plus untouched original bytes.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) preserves original names.
 */
void static_key_text(StaticKeyWriter *, const char *, size_t);
/** Emit the whole version-one static descriptor from its frozen canonical plan.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) fixes encoding version/order.
 */
StaticKeyStatus static_key_emit(const StaticKeyPlan *, StaticKeyWriter *);
#endif
