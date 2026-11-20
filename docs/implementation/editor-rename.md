# Identity-aware Ama name edits

T108 builds on [editor snapshots](editor-analysis.md) and exposes
[the rename API](../../src/editor/rename/rename.h). A rename selects one original
variable, interface field, constant, foreach-local or static graph declaration. It edits only
source occurrences resolved to that declaration. Equal text in another namespace,
a literal, a comment or an unrelated scope keeps its original bytes.

## Complete analysis domain

`editor_workspace_build` owns a copy of the configured root kinds, paths,
exclusions and entry. It constructs the catalog, classifies every source with the
production layout/activation parser, and passes the sorted active module paths
as explicit requirements to the ordinary compiler frontend. This includes active
modules that the selected entry never imports. Drafts remain opaque under
[SYN-04](../../spec/language/source-and-names.md#syn-04); their original bytes still
belong to the version checkpoint. An invalid unused active module blocks complete
workspace publication. Entry-only analysis cannot certify a workspace rename.

Prepared-catalog project/frontend entry points reuse the ordinary required-module
registry, parser, semantic stages, input graph and resolved planning. Explicit
`module_catalog_override` copies candidate UTF-8 text before any AST/native
consumer exists. It preserves logical identity, physical path, device and inode;
failed publication preserves the selected record and retains its diagnostic.
This supports in-memory validation without staging edits into user source files.

## Prepare, validate, publish

1. Initialize an `EditorRename` with explicit allocator authority. Keep its
   complete original workspace alive until plan destruction.
2. Select a variable occurrence by exact module identity and original byte
   position. The checked index supplies its declaration identity.
3. Validate the whole replacement with production UTF-8/layout and complete-name
   scanning. Names have no leading/trailing/repeated spaces, storage suffix,
   punctuation, embedded newline or TAB. No normalization or trimming occurs.
4. Collect only occurrences of the selected symbol. Copy each affected file in
   ordered segments, replacing complete-name intervals and copying every other
   byte exactly, including CRLF, comments, literal data and compact `&`/`[]`.
5. Validate against a fresh source catalog and the ordinary complete workspace
   pipeline. Check exact source versions, declarations, bindings and control
   metadata as described below. This step writes no source files.
6. `editor_rename_publish` performs a new validation immediately before calling
   the client's one all-file document transaction. Failed validation invokes no
   writer; a same-name no-op invokes no writer either.

The writer is an explicit integration boundary. It receives every full-file
replacement and its expected original document/version together. Under exclusive
document edit authority, it must either apply all replacements or preserve all
documents. It must check expected versions while holding that authority; a prior
validation alone is not a filesystem lock. Callback rejection reports an I/O
failure. The core editor service supplies plans and validation. T152 exposes
[validated source-tool plans](source-tool-commands.md); filesystem application is
still the client's atomic document transaction, not a write performed by the
read-only plan command. The composition test implements an actual all-or-nothing in-memory editor
document store, with all rejection points before infallible publication.

The plan owns replacement bytes and current candidate analysis. Its original
workspace stays unchanged. Validation diagnostics borrow the current candidate
and expire at the next validation or destruction. Writer consumers copy returned
bytes if they need them after plan destruction. Further analysis uses a fresh
snapshot corresponding to the client's new document version.

Validation covers the compiler's Ama source, semantic and planning stages.
Native compilation and exported-client rebuilding remain build steps; the edit
service exposes statically resolved Ama names and exact source changes. Module
relocation and its path/configuration transaction are separate tooling operations.

## Exact version checkpoint

Before installing candidate text, compare the fresh catalog with the complete
original catalog: same cardinality, logical identities, physical paths,
device/inode identities and original bytes. Added/deleted modules, a changed
opaque draft or a replaced file invalidate the plan. No timestamp or hash guess
stands in for original-byte equality.

Install each prepared replacement into the fresh catalog, analyze all active
modules, and compare native C/header/ABI file inputs with the original sealed
compiler input snapshot. An unplanned changed tracked native dependency rejects
publication and identifies its path in the diagnostic subject. Replacement Ama
bytes are intentional; native companion edits are not part of this operation.
The `changed-input` diagnostic is distinct from a semantic `rename-conflict`.

## Why successful compilation is insufficient

An existing compatible reference can change a foreach binder from a new local
into reused storage while leaving the candidate well typed. Name inference can
also stop introducing a persistent field after a name is captured. Compilation
alone therefore does not certify a rename under
[NAME-06](../../spec/language/declarations.md#name-06).

Let `T` map original source boundaries through the ordered edit intervals, and
let `D` map old declaration identities to candidate declarations. Validation
requires:

- `D` is a bijection: every persistent/local declaration retains its mapped
  source range, owner/type identity, role/ordinal, reference/list form, inferred
  status and lexical foreach scope;
- every indexed occurrence `o` has exactly the mapped source interval and
  `binding_after(T(o)) = D(binding_before(o))`;
- each module keeps its activation/copy/storage metadata and all constructor
  source/transfer plans;
- every flat statement, ordered binding/action, condition selection, traversal
  introduction/reuse decision and control/return alias retains its mapped
  metadata; constant startup order retains the same declaration correspondence.

Only the selected declaration's base name changes. All other bytes are copied
verbatim. Every replacement is a nonempty complete name, so token-boundary order
is preserved. Mapping rejects an endpoint strictly inside a replaced token.
Together, the checked compiler result and these finite graph equalities establish
alpha-renaming for the statically resolved Ama program, including unchanged uses
that could otherwise be silently captured. No spelling fallback repairs a failed
correspondence.

Source and edit coordinate indexes use exact ordered binary lookup. Concrete owner correspondence compares complete nominal capture graphs, including
original formal identities and residuals; matching module paths alone is insufficient.
An explicit iterative memoized work list expands each term pair once. A static
formal rename changes its mapped source name, not the correspondence of its origin.
Every old owner must have exactly one candidate owner, and the mapping must be
injective. Declaration correspondence then requires the mapped owner, Type and
original source position; exact finite scans avoid discovery-order assumptions.
Occurrence alternatives are compared as sets with exactly one mate in both
directions, retaining source-only and all concrete evidence.

For T original/candidate owners, Q vertices in the largest compared term-pair
graph and D declarations, these direct exact scans take O(T² Q² + D²) time
and O(T + D + T² Q) candidate-owned scratch space in the worst case. Memoized
pairs are bounded by the finite product of the input vertex sets. No depth limit,
name fallback or hash-only equality changes the answer. Control aliases compare their unique AST source identity without
recursively following cycles. Whole-catalog membership checking uses finite exact
catalog scans. All source/index/replacement memory has explicit owner teardown;
every allocation-failure prefix leaves source documents untouched.

## Executable evidence

`make -C tests/unit/editor/rename` has four component/composition leaves:

- `plan`: callee labels versus same-spelled caller values; unused importing
  modules; qualified constants; Unicode replacement names; CRLF/comment/literal
  preservation; compact reference/list suffixes; same-name no-ops; branches,
  loops, selected-method refusal bodies and intrinsic labels versus index names;
- `validation`: invalid complete names, actual compiler collision diagnostics,
  successful-compilation silent capture, allowed disjoint local/later-inferred
  names, changed source bytes and changed catalog membership;
- `composition`: one versioned all-file transaction, rejection preserving every
  document, fresh checking immediately before publication, no writer after failed
  validation, changed authentic native text companions, retry after allocation
  failure and every measured workspace/plan/candidate allocation ordinal.

`tests/unit/editor/workspace` checks unused active sources, opaque drafts,
invalid unused modules, copied transient root settings and every measured
workspace allocation prefix. `tests/unit/source/module` directly checks explicit
candidate text publication, retained old source views, exact physical identity,
UTF-8 rejection and each override allocation failure. The full editor tree and
source catalog leaf are exercised under ASan/UBSan. Per-file named cases and
standalone headers are recorded in [ready.json](documentation/ready.json).

## Static source names and multiple concrete captures

T192 replaces the single concrete-symbol target with an original source definition
view. Ordinary replacement names remain complete base names; static replacement
names must include exactly their three-dot prefix and pass the shared production
static-name recognizer. No implicit prefix is inserted. Graph declarations,
first/equality receipts, free formals and foreign actual labels retain their own
source namespaces. Open ordinary templates can be renamed without fabricating an
executable Type.

Selecting a shared higher-order label with distinct actual formal origins is
ambiguous and refused. Selecting just one constructor formal is also refused if
its edited label would affect another actual origin at the same source interval.
The service does not invent a coordinated rename of distinct declarations.

The `static` leaf exercises full Type correspondence for multiple captures,
Unicode composite formal names and constructor labels, named graph outputs and
ordinary selectors, uninstantiated fields, invalid whole names, multi-origin
refusal and every measured static candidate allocation failure. Existing
transaction/native-version checks remain in `composition`.
