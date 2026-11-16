# Reusable source-backed editor analysis

T107 exposes [the editor API](../../src/editor/analysis/analysis.h) without adding
another parser, resolver or type system. It consumes the production frontend's
original bytes, AST, checked symbols and resolved call/binding metadata under
[COMP-02](../../spec/implementation/compiler.md#comp-02).
No C compilation, native execution or source rewriting occurs. T152 exposes these
observations through the [explicit source-tool commands](source-tool-commands.md);
clients can also retain these C API owners directly.

## Snapshot lifetime and status

Initialize an `EditorAnalysis` with explicit allocator authority, build once from
stable configured roots and an exact logical entry, query it, then destroy it.
All returned spans, signatures, diagnostics and symbol/type identities borrow
that owner. Filesystem inputs must remain stable while a snapshot is constructed,
as for the compiler frontend. To analyze an edit, construct a fresh owner. Old
snapshots retain their original copied bytes even after the physical file changes;
a consumer must select the snapshot corresponding to its document version.
This API does not silently mutate a cache or claim unsaved-buffer synchronization.

A complete analysis exposes only fully checked semantic metadata. A draft remains
intentionally opaque under [SYN-04](../../spec/language/source-and-names.md#syn-04):
lexical highlighting is available, but no guessed declaration graph is published.
Invalid active source retains the compiler diagnostic and may still expose valid
lexical prefix tokens. Missing/catalog/allocation failures leave an unavailable,
safely destructible owner. A false build never sets `indexed`.

The selected source is inspected from the same retained catalog record when
strict checking fails; it is not reread from a possibly changed physical file.
All partial allocations have the same reverse destruction path. Semantic/index
views are consumed before frontend/source storage disappears.

## Queries

| API | Result and boundary |
| --- | --- |
| `editor_highlights` | Production layout/lexer categories and exact source/literal spans. A temporary layout is destroyed before return; source spans survive with the snapshot. |
| `editor_definition` | Exact checked symbol/type and original declaration spelling at a half-open byte interval. A module identity targets its source-file start; absent matches preserve the destination. |
| `editor_completions` | Exact names visible at an executable source point, with their checked type, reference/list shape, role and declaration. This is lexical availability, not ranked suggestions or grammar repair. |
| `editor_signature` | Declared input/result/constant fields of an exact checked module type. Private state is not offered as an external interface. |
| `editor_operation` | Checked call metadata at its written target, including intrinsic list service/method descriptors without fabricated nominal types. |
| `editor_binding` | Callee field/type/storage at a written label or positional marker; intrinsic fields retain their static interface descriptors. |
| `editor_diagnostics` | First compiler error plus valid AST warnings, with original primary/related locations and explicit warning severity. Selected-source fallback inspection never duplicates warnings. |

Highlight callback records are temporary; copy their spans/values to retain
observations with the snapshot. Callbacks are synchronous borrowed observations. Returning false stops an
enumeration successfully; it does not consume the snapshot or report an error.
Callers must not destroy/mutate the owner inside a callback. Byte offsets refer
to original UTF-8 bytes, not UTF-16 editor columns or normalized display strings.

Highlighting uses the same complete-name and punctuation scanner as parsing.
Physical layout now retains a comment span alongside its existing code/payload
spans. Comment spans include their preceding separator/indentation, while `#`
inside literal data remains payload. Repeated literal/comment spaces and CRLF
bytes are preserved. Invalid tokens stop at the actual lexer diagnostic; the API
has no alternative spacing grammar or heuristic recovery.

## Identity index and visibility

The index walks checked symbols, initializers and the already flattened semantic
statement inventory. It records declaration names, explicit type paths, accesses
and nested indices, qualified constant namespaces/members, caller receivers,
callee labels, selected returned-method targets and foreach bindings. Synthetic
output-transfer sources intentionally have no source syntax: their actual written
label and receiver supply the occurrences instead. Positional transfers invent
no named callee spelling.

Each occurrence retains its exact semantic identity. Equal spellings in different
owners or disjoint foreach scopes remain distinct. Duplicate aliases at the same
source range must agree on identity; disagreement fails indexing rather than
choosing a plausible target. Iterative binary-bin merge sorting costs O(R log R)
for R collected records, uses bounded nonrecursive merge administration, and
produces deterministic physical-path/byte-range order. A final pass merges only
identical records, preserving a declaration flag. Arena storage is O(R).
Definition/call/binding queries scan finite retained inventories without allocation.

Completions call semantic inventory predicates under
[NAME-06](../../spec/language/declarations.md#name-06). Explicit module fields are
visible throughout their module. Inferred fields appear only at their introducing
receiver and later points. A newly introduced foreach reference is available
only inside its body; a later same-named persistent field remains a different
identity. Constructor source points exclude runtime locals and use the same pure
initialization-order/constant-only predicate used by compiler diagnostics.
No editor-specific symbol resolution or declaration-order policy is copied into
an independent grammar/type system.

Intrinsic methods have no source module or declaration to navigate to. Their
checked operation and binding expose `SemanticListInterface`,
`SemanticListMethod` and `SemanticListField`; `semantic_list_position` supplies
the same fixed signatures used for compilation. External module signatures use
the original independent input/result roles and ordinals.

## Executable evidence

`make -C tests/unit/editor` composes three explicit C leaves:

- `lexical`: complete multiword names, inline/indented comments, raw literals,
  CRLF, intentionally incomplete drafts, invalid active code and callback stop;
- `bindings`: caller/callee separation, returned references, qualified constants,
  selected-method targets, independent input/result signatures, nested list
  indices, intrinsic methods, foreach scope and later same-named inference,
  forward explicit fields and constructor ordering;
- `composition`: Unicode changed-file snapshots, retained old definitions,
  empty-block warnings, missing roots, temporary highlighting allocation failure,
  and every measured allocation ordinal during frontend/index construction.

The lexer layout leaf directly checks the newly retained comment ranges. The
semantic visibility extraction preserves the existing compiler diagnostic text
and is exercised through the constructor and scope cases as well as ordinary
semantic suites. ASan/UBSan checks all editor leaves; strict full-project checking
covers the shared lexer/semantic structures and backend consumers. Per-file cases
and standalone headers are registered in [ready.json](documentation/ready.json).
