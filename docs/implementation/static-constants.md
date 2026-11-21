# Dependent constant Types and concrete namespaces

[T212](../../DONE.md#t212) aligns constant metadata with simple ordinary Type
forms and named static exports. Authority:
[TYPE-01](../../spec/language/declarations.md#type-01),
[CONST-01/CONST-02](../../spec/language/declarations.md#const-01),
[CONST-03](../../spec/language/declarations.md#const-03),
[CONST-05](../../spec/language/declarations.md#const-05),
[CONST-06](../../spec/language/declarations.md#const-06),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-11](../../spec/language/templates.md#tpl-11),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
Historical T181 evidence remains in [DONE](../../DONE.md#t181), rather than an
alternative supported interface. There are no inline static arguments or result
bindings inside ordinary declarations or constant reads.

## Boundary and ownership

The constant plan borrows [publication ownership](static-publication.md), which
owns reusable [ordinary Type obligations](static-ordinary-types.md). Destroy
constant records first, then publications, results, registry, guards, applications,
solver, terms and source inventory. Constant records own no runtime storage,
initialized values, native payloads or mutable instances. Initialization allocates
nothing; every failure leaves partial descriptors singly owned and public outputs
unchanged. Diagnostics retain the original source location and first cause.

Bind exact original shared declarations in their actual ordinary incoming scope.
The interface is already collected, so a Type parameter first appearing in an
ordinary output is visible to the earlier shared section. A shared declaration
cannot introduce a new parameter. Its Type is a concrete/simple module path,
existing Type parameter, or one independently concrete named static export.
Scalar and list storage remain separate from the element Type; reference and
reference-list shared storage are refused before preparing Type work. Literal
bytes add no Type evidence and never select a specialization.

Registration retains a symbolic consumer Type variable and its original ordinary
Type record. It does not resolve or execute an initializer. Type constraints may
remain pending until permitted typed sources supply evidence. A static export,
however, must close its entire supplier independently before any consumer equality
is admitted. Constants use the same operation as ordinary fields instead of
constructing a nominal producer Type or reconstructing call syntax.

## Explicit and qualified namespaces

An explicit saved read retains original namespace syntax and raw named/positional
ordinary `>>` requests. It selects no member while the namespace is unknown.
Final selection independently resolves a complete ordinary Type, then normalizes
requests against that Type's own `@=` declaration surface. A named static export
is read before the ordinary member requests. The selection retains canonical
`namespace_type` metadata; the read retains its ordinary source/supplier proof.
A supplier bundle is not an ordinary namespace value.

Requested names, positions, list labels, typed receivers and literal payloads
cannot complete a namespace. No signature search, default instance or matching
module guess is permitted. A bare supplied Type can name the namespace; a residual
Template cannot. Only own shared declarations participate: input/result variables,
body fields and constants of argument Types are excluded. Duplicate own base names
and unselected shared reference storage fail before first-match lookup. Runtime
storage labels and named/positional order remain ordinary binding rules.

A plain fixed/supplied Type requires an explicit actual checked ordinary context
from the required-type registry when source work supplies none. Check its complete
canonical identity only after independently resolving the namespace. A supplying
static-module context has no ordinary owner value and is refused before a null
term can reach the solver. Independently naming a Type does not certify its
ordinary body or create an instance. A previously returned selection rechecks
current equations, scope and guard provenance before reuse.

Qualified sources retain exact original AST/catalog/dependency occurrence.
T232 keeps the whole raw operand in the AST and selects the namespace root kind
and final member only after defining-scope alias expansion and normalization;
the loaded dependency owns that descriptor. Member bytes are independent of
optional source provenance, including whole aliases and inherited components.
Ordinary semantics consumes the selected declaration proof rather than
reconstructing a raw member span. Their written namespace has no static
binding syntax. An open template cannot be completed by the desired member or
destination. Select own shared storage only after concrete namespace closure and
an explicitly checked matching ordinary context. Preserve fixed-Type reuse
provenance without adding a constructor creation edge. Same text at a different
source occurrence cannot replace missing dependency evidence. Bare single-dot
names remain atomic Type placeholders, not qualified value sources.

## Typed initializer evidence

Register only original shared initializers. Local value sources must refer to
own shared declarations; qualified sources retain their fixed namespace/read
obligations. Literal sources have no intrinsic Type. Mutable references, self,
continuations and runtime names cannot supply constant initialization evidence.
Registration retains source descriptors without executing or copying values.

Resolution advances the destination and typed source through their ordinary Type
operations, then adds their exact element-Type equality. This can infer a normal
Type parameter, including an output-introduced list element Type and nested
canonical Types supplied by a static assembly. It cannot infer a static assembly
from a destination or initializer: its full producer closure precedes that equality.
Source/destination storage/cardinality, constant capability, construction order,
cycles of runtime constant values and native lifecycle remain later owners.
READY here concerns structural work plus the closed declared Type, not initialized
values or executable admission. Intermediate Type facts remain available while
ordinary nominal-group body validation is pending.

## Invariants, termination and costs

Every declaration/read/source/initializer record has exact original syntax and
actual context membership. Every final selected member belongs to the independently
closed namespace's own declaration sequence. Substitution uses original symbols
and canonical Type identities, never text replacement. Public handles are only
linked after their required preparation succeeds; partial arenas stay owned on
failure and no rollback of valid solver facts is promised.

Registration and own-member checks are finite list scans. Duplicate own-name
checking is O(D²) for D declarations. Surface preparation uses O(C) fixed-capture
storage; selecting R requests scans the finite surface and uses the existing exact
binding contracts. Metadata record lookup is linear in retained record count.
No operation recurses through runtime values, searches candidate Types or expands
an unknown producer. Structural solver/publication costs and termination are those
of their separate guarded components. This layer submits finite original Type
obligations; it introduces no retry/depth budget or heuristic.

## Acceptance boundary

The `constants/{declaration,read,selection,failures,qualified,catalog,initializer}`
leaves cover symbolic/applied/shared Types, output-introduced parameters, exported
scalar/empty-list Types, raw reads, own surfaces, independent exports, exact
required scopes, root/member spans, fixed-Type reuse, nested typed copying,
literal noninference, malformed identities, counters and measured allocation
refusals. Per-file/header evidence belongs to [ready.json](documentation/ready.json).
The completed T212 record retains strict/sanitizer/build/documentation commands,
results and the unavailable-LSan boundary.
Runtime typed connections are T182; ordinary admission and actual constant value
initialization/lifecycle remain T184/T185/T186. Historical acceptance of removed
syntax is not current coverage of those tasks.
