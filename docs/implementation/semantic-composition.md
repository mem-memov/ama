# Complete semantic composition

T040 checks the direct concrete-source semantic composition under
[COMP-02](../../spec/implementation/compiler.md#comp-02),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-05](../../spec/implementation/compiler.md#arch-05),
[ARCH-06](../../spec/implementation/compiler.md#arch-06) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
It composes the individually verified T030–T039 components; it does not claim
that later IR/runtime/emission work is complete.

The [T185 specialized composition](specialized-semantics.md) feeds closed
specialization descriptors into these ordinary checks, retaining full capture
identity and actual incoming evidence. It separately certifies finite candidate
groups before the frontend requests constant and resolved plans. The source-only
six-stage sequence below remains the independent concrete component/relocation
entry; it does not repeat inference for a projected closed inventory.

## One explicit stage sequence

[The analyzer](../../src/semantic/analyze.c) receives a fresh semantic owner
borrowing a valid loaded project. It runs exactly this prerequisite sequence:

1. Complete explicit names and declaration roles.
2. Real required native companions and six-slot ABI contracts, with an optional
   explicitly supplied authoring proposal applied only to its exact module.
3. Source-order receiver inference and lexical foreach-role classification.
4. Complete transfers, ordinary/intrinsic calls, selected actions, control values
   and positive copy-boundary proofs.
5. Independent eager-ownership and actual constant-read/order relations.
6. Deterministic shared startup and reverse cleanup plans.

Inference is now an explicit composite step. `semantic_resolve` consumes that
successful result without re-running classification. Component users can still
call resolve after native checking: its existing prerequisite path runs inference
when no attempt has occurred. A composite inference failure therefore leaves
`resolve_attempted` false instead of pretending full resolution began.

Each component publishes its own successful flag and retains its first original
diagnostic on failure. The composite short-circuits at that failure; no later
component consumes failed or partially initialized output. The complete success
gate is `constants_valid`, after every preceding valid stage. The analyzer never
writes output, runs a module body, executes native constructors or inspects C
source for callable signatures.

## What is resolved before lowering

At successful completion, metadata records all language interpretation choices:

- names identify exact module/symbol identities, including root kind, or a fixed
  intrinsic interface descriptor;
- sources retain their explicit kind, exact type/reference/list form, original
  span and selected transfer kind;
- ordinary bindings retain interface roles/ordinals independently of caller text
  order and explicit omission;
- selected targets retain a parent field or intrinsic result ordinal, and every
  output action links its already checked binding/suboperation;
- predicates retain their own native conversion type, exact header restart root,
  terminal owner and incoming lexical loop; foreach retains its collection/cell
  role, with branch polarity and syntactic body emptiness explicit;
- constants have actual construction dependencies, and startup/reverse plans
  refer to individual declarations rather than merely dependent module types;
- statically positive copy identity/ownership violations have been rejected.

A runtime handle's liveness, generation, availability, index result or saved
position remains runtime state, not an unresolved interpretation. The same is
true of native literal acceptance and native resource failures. Callable C
signatures, target payload layout and the fixed integer size bridge remain typed
native-build obligations rather than guessed source meaning.

## Semantic validity and executable support

[Frontend completion](../../src/compiler/frontend.c) consumes complete semantic
success before initializing the plan owner. T041–T044 now preserve current source
forms through [complete structured IR](structured-lowering.md). The frontend
publishes only a validated full plan. [C generation](../../src/codegen/project/generate.c) consumes this plan and emits
complete adapters, including [live traversal](list-traversal-execution.md).
Handwritten native bodies require the subsequent C compilation/verification;
static semantic success alone does not validate native implementations.

Failed semantic programs remain safely destructible, with valid source-backed
diagnostics, until the caller destroys the semantic owner before its project.
Partial graphs, proposals and proof records do not become alternative successful
results. No heuristic repair or source/type fallback makes an invalid program pass.

## Composition law and verification

Treat each stage as a partial function whose domain requires all predecessor
valid flags. Successful composition is their ordered function composition.
Induction over the stage sequence establishes that each invoked consumer receives
its documented complete input. If a stage fails, short-circuiting preserves that
failure and prevents every later attempt. Initialization credits/arena ownership
cover exactly the successfully allocated prefix, independently of the failed
stage. The sum of the component termination/memory bounds therefore bounds the
composite; composition adds a constant number of dispatches and no recursive
source traversal or native work.

Run `make -C tests/unit/semantic/composition`. The complete real project combines
constants-only namespaces and qualified defaults, raw literal bytes, self results,
reference inputs, inferred fields, selected entry/resume siblings, all list-service
result forms, foreach locals/escapes/yield, a command-header loop, negative-only
branch and continuation copy. Independent expected assertions count/control these
features and verify actual constant order, exact literal bytes and integer bridge
obligation. An empty executable project is also accepted without inventing fields
or constants.

Two independently allocated semantic owners over the same immutable loaded
project are then analyzed through the composite API and the explicit stage APIs.
The test compares exact semantic projections: type/clone causes, symbols and
scopes, ABI spans, initializer kinds, every source/access, binding/action order,
parent/loop/result identities, traversal roles, constant ordinals and reverse
links. It compares shared original AST/source identities and complete byte ranges;
semantic allocation addresses, struct padding and hash equality are never used
as substitutes for result equality.

Six invalid real projects fail in names, native ABI parsing, source inference,
full transfer checking, eager graph ordering and positive alias checking. Both
paths must retain exactly equal diagnostic categories, all optional prose fields,
primary spans/paths and related spans/roles/names. Attempt flags must identify the
same failed prefix. Each semantic allocation in the complete valid project is
independently failed, including late constant-plan publication; every failure is
explicit out-of-memory with safely owned partial output and zero leaked blocks.
Authored C companions deliberately contain `#error` to make accidental native
compilation observable. Full strict and sanitizer runs supplement the component
and composition invariants.
