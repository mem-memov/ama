# Ordered entry preparation and value replacement

T043 makes the replacement protocol explicit in the typed plan under
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[REF-03](../../spec/language/values-and-references.md#ref-03) and
[COMP-02](../../spec/implementation/compiler.md#comp-02).
The [specification](../../spec/README.md) defines the observable semantics;
this chapter describes the compiler records and their tested ordering.

## One recipe, one construction path

A [replacement recipe](../../src/planning/replacement.h) describes one assignment,
parameter installation, result initialization or output transfer. All instructions
for that replacement share the same arena-owned recipe. It retains exact typed
sources, receivers, field identities, ownership/cardinality and copy mode. It does
not contain generated C strings or a copied runtime value.

Construction chooses exactly one path:

| Path | Source context | Effect |
| --- | --- | --- |
| Supplied source | Initiating caller | Consume the selected source using the checked ordinary, reference-handle or continuation mode |
| Supplied literal | Raw literal payload | Construct with the selected native literal capability; never construct a default first |
| Declared default | Callee instance and field initializer recipe | Use that field's initializers/default once, after earlier interface fields have committed |
| Retained result/shared value | Callee retained interface or immutable namespace | Transfer into the caller receiver selected at this action |

The source context identifies where values are selected; a native constructor
still constructs its actual destination value under the lifecycle contract.
Standalone self references retain their existing explicit runtime guard pending
final construction/clone identity work. No source is silently rewritten into a
snapshot to make an invalid overlap succeed. Native reference handles remain
nonowning handles and do not acquire their referents merely by being copied.
Literal payloads, including apparent operators/comments and trailing spaces,
remain exact bytes. This representation adds no source syntax: literal data is
accepted only in the positions allowed by the grammar.

## Entry preparation has three distinct orders

[Call lowering](../../src/lowering/calls/operation.c) first acquires the target.
Only that selected target's refusal follows the operation's lexical `:` branch.
Source selection, conversion, preparation and cleanup failures retain the fatal
capture-before-cleanup route even if their status code resembles an acquisition
refusal. Routing uses failure provenance, not a guessed numeric classification.

For entry calls, every supplied ordinary source/index is selected in binding text
order. A literal needs no runtime location selection; its construction belongs
to installation. `CONTROL_PREFLIGHT_INPUTS` then checks the affected interface
against all protected supplied sources before the first reset. It exists even
when the supplied-input sequence is empty. Input protection preserves the exact
selected boundaries without copying payloads as temporary snapshots.

Parameters install in declaration order, choosing the supplied source/literal or
one default. Results initialize afterward in their declaration order. Permanent
body fields are excluded from reset. A default that reads an earlier parameter
therefore sees its installed state; a supplied first parameter cannot run its
unused default's constructor or other effects. Noncopyable omitted/defaulted
storage does not acquire a fictitious copy-capability requirement.

Only after successful installation does the caller set the return connection
and enter the callee. Resume omits all source preparation and reset and enters
the saved position. Retained reads omit reset and body/return connection;
constant reads also omit acquisition and release. These modes are separate graph
shapes, not runtime guesses based on input counts.

On completion/yield, the return continuation processes requested output bindings
in binding text order. Each receiver, including its current index, is selected
at that binding's own action. Two different results may replace the same receiver
in sequence; they are not simultaneous. An omitted output creates no replacement.
The target remains protected until the output sequence and cleanup release it.
Selected subtrees and foreach execution remain T044 and later work; their current
unsupported-feature guards still prevent incomplete lowering from reaching C.

## Explicit replacement phases and failure meaning

[Replacement lowering](../../src/lowering/replacement/recipe.c) emits these groups:

| Context | Ordered instructions |
| --- | --- |
| Assignment | Select receiver; select source; preflight; prepare fresh replacement; commit; release |
| Parameter/result | Installation/initialization site; preflight; prepare; commit; release |
| Output | Transfer site with current receiver selection; preflight; prepare; commit; release |

Every recipe names its first instruction, commit point and final release. The
supplied source was selected before reset, so installation does not repeat its
index conversion. Defaults instead use the callee's current installed prefix.
Output sources name retained interface storage and their receivers are selected
only at the output action. Release hands off that replacement's staged values and
source access; it does not release unrelated pending inputs or the enclosing
operation's protected result target.

Preparation failure keeps old receiver storage unchanged. Commit is a distinct
irreversible boundary; cleanup/destruction failure after it is terminal. Earlier
successful field or output commits stay committed if a later replacement fails.
The graph has no whole-call rollback or restore instruction. All technical failure
edges capture the active operation before cleanup, preserving the first failure.
Runtime lifecycle ledgers retain the successfully constructed prefix and live
payload state needed to clean partial construction correctly.

## Validation and the current C adapter

[Operand validation](../../src/planning/validation/ir/replacements/operands.c)
compares each recipe with the owning semantic statement, exact field/binding,
source/receiver, construction choice and transfer mode. [Group validation](../../src/planning/validation/ir/replacements/groups.c)
requires its complete finite phase sequence, shared ownership, commit/release
endpoints and fatal routes. Every phase has exactly one recipe. Entry, branch and
resume edges cannot jump into a group's interior. [Invocation validation](../../src/planning/validation/ir/invocations.c)
checks source text order, aggregate preflight before reset, declaration-order
installation and output text order without following callee bodies or runtime
backedges. Validation allocates no scratch storage and is quadratic at worst in
the finite plan inventory.

The existing C adapter executes a contiguous entry-preparation prefix through
`ama_program_prepare` at acquisition and a contiguous replacement through its
lead runtime helper. Later phase labels expose the typed plan and must not repeat
those effects. This grouping relies on the same verified operation order, no
interior branch/resume entry, no Ama suspension inside native construction hooks,
and the runtime helper's precise original source-site reporting. It introduces no
C activation surviving an Ama transfer. Runtime helpers already separate target
refusal provenance from preparation failures and preserve earlier commits.

The block invariant is that a replacement starts from a selected/authorized
receiver and required source, reaches its continuation only after the one chosen
construction commits and cleanup releases its temporary state, or takes the
specified fatal edge. Backward prepending constructs this sequence without
recursion. Induction over parameter/result inventories and output bindings then
proves their respective orders. No induction follows the runtime call graph, so
same-type recursive calls remain finite compiler work.

[Transfer tests](../../tests/unit/planning/transfers/main.c) load real typed
projects and check reverse named-input text versus declaration order, defaults
reading installed parameters, omitted noncopyable references, exact literal
bytes, repeated output receivers, invocation modes, ordinary/handle/continuation
assignments, reference results, broken recipes/phase edges and every lowering
allocation failure. Existing compiled execution suites check the current helper
adapter, actual lifecycle effects, source traces and refusal/fatal separation.
