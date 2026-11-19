# Guarded static result equations

[T211](../../DONE.md#t211) implements structural result evaluation for the named
`@...` graph. Authority:
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[ARCH-03/ARCH-06](../../spec/implementation/compiler.md#arch-03).
Historical T179/T201 results remain in [DONE](../../DONE.md); obsolete ordinary
static sections, positional static bindings and output skips are removed.

## Representation and ownership

A result plan borrows an initialized [canonical registry](static-registry.md)
and its guards, applications, solver, terms and inventory. Destroy results before
those owners. The result arena owns source/context/use/definition/graph/cache
records and immutable prepared producer maps. Evaluation scratch belongs to each
attempt and expires on success, Pending or failure. Diagnostics borrow original
source data; a failed owner remains destructible but cannot resume.

Original `(environment, expression)` identifies a source. Ordinary constructor
and supplied Type/Template expressions have a `head` and separate `value` term.
A named static producer statement has its actual `producer` origin and null
`head`/`value`. It cannot be converted into a nominal Type, nested metadata value,
or ordinary instance. Exact registered occurrence membership precedes syntax
pointer dereference.

Pair results and first-receipt aliases have separate derived variables and exact
original defining syntax. Later receipts impose equality without replacing that
definition. Actual use keys include incoming creation context and definition
ancestry, independently of shared source/operand metadata. A result context
retains its actual registry use; cached lookup refreshes guard provenance.

Named producer preparation normalizes the actual foreign interface. Explicit
input expressions supply lexical child value terms. Every omitted foreign input
gets a stable inference identity `(lexical environment, original expression,
original formal)`, not a new free caller parameter. The full input array follows
original formal order for canonical identity; source binding order is not a
positional interface. Repeated preparation reuses the record. Unknown/repeated
labels retain the binding diagnostic. Zero inputs need no fabricated value.
The ordinary [operand cache](static-operands.md) remains private and immutable;
sharing preparation never certifies a new incoming path.

Before application, whole-environment registry canonicalization shares complete
variable identities only after the owner key is proven equal. This includes
result, expression and omitted-input variables, with no hand-maintained role list.
Otherwise equal zero-input producers could infer different context-dependent
outputs. Only environments change in the identity; supplied captures remain exact.

## Separate creation, definition and validation graphs

Guarded entry compares complete original input maps before publishing another
producer scope. Unknown repeated keys suspend, unequal keys fail, equal keys may
close recurrence. The result context is exposed only after this boundary. A
normalization query may intern canonical metadata; it must not instantiate a
rejected raw scoped request.

Actual source-symbol roles determine value reads. A local result or alias reads
its own definition in the current context. A supplied parameter remains metadata
even when its captured underlying term is a foreign derived variable; evaluation
does not search unrelated contexts for a similarly named definition. An alias
reads the exact foreign slot selected at its original first receipt, under the
alias's own definition path. Receiver equality adds no definition-read edge.
Zero-binding supplied values and static partial expressions obey `head = value`
for both Type and Template, including before their category is known. An ordinary
FIELD-policy generic module path instead instantiates source-owned omission
variables under TPL-04; its field Type is not the residual constructor. Neither
operation supplies a guessed origin. [T217](../../DONE.md#t217) covers this
contextual distinction and source-only ordinary annotation consumers.

Whole-producer validation is queued independently. Consider `.a`:

```text
@...
.b
 ...selected > ...out
```

and `.b`:

```text
@...
...selected
..integer
.a
 ...out > ...unselected
```

The creation/validation graph is A → B → A with equal zero-input keys. The value
reads are B.unselected → A.out → B.selected, an acyclic chain grounded at integer.
It must succeed. Making A.out depend on every B output would invent a cycle.
Conversely, replacing B.selected's definition with A.out creates a real value
cycle and fails even if another selected output is concrete.

Each actual context can own a graph witness containing all pair/alias definition
roots and all producer statements, including equality-only statements. Every root
uses its original registered work handle, not a reconstructed root with different
ancestry. A same-key recurrence links to an already required actual ancestor
witness. Independent incoming paths replay when necessary: the globally first
witness may have unresolved ancestors and cannot suppress a path able to derive
inputs. Closed metadata variables are shared; proof provenance is not.

A graph pass advances roots with the explicit value-evaluation stack and marks
its local certificate only after all local work and its own full key close.
Required child graphs remain separate obligations, so validation groups do not
recursively wait on themselves. Ordinary Type/body checks remain pending registry
obligations; local graph completion is not executable admission.

## Closed definition cache and publication

Cache keys are `(complete canonical owner environment, original symbol)`, including
compiler-only producer maps. No ordinary producer Type is fabricated. Cache store
requires checked RHS work, complete owner identity and selected value; pair values
must equal their defining ordinary expression value. A named producer RHS has no
own value, and its closed flag includes all producer inputs and selected values.
Returned ordinary Template residuals remain local binders.

A cached value does not erase its checked original RHS. Reuse in the actual
context keeps that proof; reuse from another context additionally requires all
actual ancestor keys closed. Otherwise the original RHS is replayed under the
new path. Guard entry and refresh precede hits; cold and warm orders preserve
acceptance/refusal. [Publication](static-publication.md) follows retained source,
definition and graph proofs before exposing immutable results.

## Correctness, termination and checks

Exact source/slot/formal identities preserve lexical substitution. Canonical
scope sharing requires proven complete equality. Guards prevent expansion through
unknown or unequal repeated origins, while exact definition paths reject true
value cycles. Graph witnesses and source-value facts remain different proof
objects; neither a receiver equation nor cached metadata substitutes for producer
validation.

Expanded witness chains have no repeated origin: unknown recurrence suspends,
unequal recurrence fails, equal recurrence links to an actual ancestor. Finite
roots and finite source branching imply finite witness work, potentially
exponential simple-path expansion. Every value walk uses an explicit stack and
unconditional scratch teardown, with no recursion/depth heuristic. Publication's
finite evidence measure governs retries. Linear exact membership/interning scans
retain their documented component costs; no source-linear whole-graph bound is
claimed.

Current leaves under `results/` cover source/context identity, complete named
maps, guarded entry, independent graph witnesses, anchored/unknown groups,
cache keys, ordinary shallow operands and iterative values. They include foreign
handles, counter boundaries, measured allocation refusals, supplied foreign
metadata and definition/equality-cycle distinctions. File-specific cases and
standalone headers are registered in [ready.json](documentation/ready.json).
Task commands, failed-run corrections and acceptance evidence are retained in DONE. Ordinary named
export/constant integration is T212; runtime typed connections and whole-project
admission remain T182/T184/T185. None is claimed by this component alone.
