# Source-order inference and foreach scope

This boundary implements [TYPE-06](../../spec/language/declarations.md#type-06),
[NAME-06](../../spec/language/declarations.md#name-06) and the static binding part
of [FLOW-05](../../spec/language/lists.md#flow-05). It follows the complete explicit
inventory from T030. It does not execute callees or infer types from runtime data.

## Separate availability from first occurrence

The explicit inventory is available before scanning any module. Its names, exact
nominal types and interface positions can resolve ordinary earlier uses. That
does not count as encountering those names at earlier source positions.

The inference pass keeps a separate encountered-name set. A first simple
assignment/output receiver declares a persistent field. An earlier ordinary use
consumes that first occurrence, even when unresolved; the pass rejects missing
types immediately rather than searching for a later receiver. Callee labels,
module paths, qualified constant members, selector base names, literal payloads
and comments do not enter that set. Caller names in indices do.

A first receiver still declares when a same-named explicit declaration appears
later. That is a duplicate: the diagnostic points at the later explicit spelling
and retains the actual earlier introducing receiver. Ordinary earlier uses can
resolve later explicit declarations without inventing a field. All four storage
spellings share their complete base name, including its internal single spaces.

## One stream of actual written constructs

[Event collection](../../src/semantic/inference/events/collect.c) follows every
owning AST subtree and creates events at declaration names, initializer sources,
assignments, operation heads, individual ordinary bindings, conditions and foreach
headers. [Stable sorting](../../src/semantic/inference/events/sort.c) then orders
those events by exact source-byte position.

An ordinary binding can lie after one selected operation and before another.
Processing all of a parent call's stored arguments before its children would
allow a later receiver to repair an earlier missing operand. Separate binding
events prevent that mistake. A standalone assignment is one adjacent construct:
its receiver is classified before its following source line, whose names are
visited in order before the next construct. No other statement interleaves them.

[The pass](../../src/semantic/inference/infer.c) applies each event once in this
order. It types a fresh receiver immediately before processing later constructs.
Explicit declarations, later inferred fields and active local references never
provide guessed alternative declaration sites after failure.

## Exact types and storage forms

Assignment sources use TYPE-06's complete table: owned/reference scalars, whole
owning/reference lists, indexed scalar selections, reference acquisition, lexical
self and explicit continuation copying. A continuation copy has owned scalar
transfer form even when its operand is a reference. An indexed reference-list
entry remains a scalar reference. Written receiver suffixes must match; no suffix,
dereference, cardinality conversion or literal type is inserted.

Ordinary call/read outputs use T030's shared declared label/position metadata.
Omitted bindings create no name and still leave later arguments at their normal
positions. A selected target resolves only through its parent's result interface;
its indices and input operands resolve in the initiating caller. Typed operation
prefixes are cached by AST identity after their targets are reached, so finite
paths require no recursive C descent or callee-body inspection.

[List methods](../../spec/language/lists.md#list-04) have a distinct compiler-typed
interface: append/insert return an element reference; length returns owned
`..integer`; the other methods have no result. These are interface entries, not
method instances or source-language action constants. The type metadata does not
create hidden objects. Every structural list-service parent contributes the
fixed `..integer` dependency at its actual target span, including direct returned
lists. That interface mentions integer positions/lengths regardless of the chosen
method. Ordinary list copies/traversals do not introduce this dependency merely
because they contain list storage. Missing Standard integer never falls back to
Project `.integer`, and diagnostics retain the real requesting construct rather
than a fabricated source file containing the implicit name.

## Locals are lexical cells, not persistent fields

At each foreach header, resolve the mutable collection first. Then prefer a
compatible enclosing local, explicit module reference or earlier inferred module
reference. An existing name must be a mutable scalar reference to the exact
full element type. Incompatible storage, constants or a different nominal type
are errors; changing suffixes cannot create a shadowing cell.

When absent, introduce one local reference for that body. Its scope is the exact
source interval after the opening marker and before the matching close. Such
intervals are nested or disjoint by grammar. Local lookup selects the deepest
matching containing interval, then ordinary module lookup applies. New local
bindings and uses resolving to them never enter the module encountered set.

Disjoint bodies can own distinct same-named locals. A later inferred module field
outside those bodies can reuse that name without changing earlier bindings;
post-inference lookup still gives those earlier body sites their original local
cells. Nested compatible reuse aliases one cell and does not introduce a second
cell or a restoration operation. Every lexical foreach role retains its own
collection/syntax and its selected cell for later runtime lowering.

Creation-time initializers cannot read these runtime locals. An executable body
assignment can copy a local handle into a persistent field, or explicitly clone
through it into an owned field. Local symbols are excluded from the module field
list, eager ownership graph and ordinary storage planning. Static binding metadata
exists even for an empty body/list; it does not allocate a runtime variable or
execute an iteration. Runtime traversal/activation remains its later feature task.

## Normal field construction and downstream consumers

Symbols now carry direct name, spelling, definition site, type site and initializer
metadata. Explicit fields retain their original declaration node; inferred fields
retain none. No fake parser declaration/type line is created. Locals keep their
foreach owner separately.

A fresh persistent field gets its exact type and storage flags, zero creation-time
initializers, and normal default/empty construction. Its written assignment or
output remains an executable transfer. Fields are merged by introducing source
position and normal module/role ordinals are recomputed. Capability/eager graphs,
lifecycle planning, namespacing, diagnostics and C generation consume that same
metadata. A later initializer can observe only an inferred field's default value;
its future assignment/call result is not turned into a constructor hint.

`semantic_infer` publishes `inference_valid` only after complete classification
and scope/type success. Normal existing-field transfer diagnostics and copy
capabilities remain in full resolution after inference; local-cell update shape
checks occur during inference. Preparing inference explicitly can be followed by
full resolution without running the classification pass twice. Failed partial
records are arena-owned and never constitute a successful stage.

Ordinary inferred assignment/call/read fields execute through existing checked
storage and transfer machinery. Direct-result tree execution, intrinsic service
execution and lexical-self transfer lowering retain explicit pre-storage planning
guards. T036–T037 now complete direct-tree/intrinsic static typing.
[T038](control-contexts.md) completes foreach/control-context checking, while
execution still requires the later IR/runtime frames.
Successful inference is not a claim that those operations execute yet.

## Invariants, termination and bounds

After every successful event prefix, the encountered set is exactly its module
name occurrences; each available inferred field was introduced at an earlier
receiver; each local lookup is fixed by the header and its body interval. A fresh
assignment's source cannot depend on the field currently being typed. Other
inferred type dependencies strictly decrease receiver position and terminate at
explicit types, fixed interfaces, an earlier typed collection or lexical self.
The explicit inventory and earlier inferred fields have disjoint valid base names.
These laws determine one classification and type without runtime information.

Each event is processed once. Structural owners are finite, aliases are excluded,
and merge passes preserve event/field multisets while doubling sorted-run bounds
without overflow. Operation prefixes are typed before their bindings/children.
Local intervals and encounter records are finite. Thus all traversals terminate
without C recursion, rescanning failed branches or a hidden depth limit.

For N structural nodes, M events/occurrences, F fields, L locals and O operations,
work-list storage is O(N + M + F + L + O), plus sources and explicit inventories.
Sorting is O((N + M) log(N + M) + F log F). Exact linear name/scope/interface and
operation indexes add their corresponding comparison costs; with maximum spelling
length W, a conservative bound is O(M(N + M + F + L + O)W). These indexes may be
replaced with faster exact structures without changing the prefix invariants.

## Evidence

`make -C tests/unit/semantic/inference` exercises the assignment storage table,
entry/resume/retained/constant outputs, labels/placeholders, whole/indexed sources,
root-kind identity, interleaved selected results, earlier unresolved names,
self-dependencies and later explicit duplicates. Scope cases cover disjoint locals,
nested/later-explicit reuse, earlier inferred lists, incompatible names/types,
local-to-field escapes and forbidden construction-time local reads. Intrinsic
metadata cases check fixed result types, direct returned lists and rejection of
method resume/copy/action-dispatch shapes. Every actual inference allocation is
failed once with zero counted resources after destruction.

The project closure leaf checks fixed implicit Standard dependency sharing, source
locations, no family fallback, ordinary-list-copy exclusion and every successful/
missing-dependency allocation prefix. The compiled execution leaf verifies fresh
assignment/output fields, default values before the first refused transfer,
creation-time snapshots before future assignments and complete module-state copying
through a newly inferred field. Per-file named evidence and standalone header
commands are registered in `documentation/ready.json`.
