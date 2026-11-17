# Ama product specification

Ama programs are built from modules that describe types and behavior, instances
that hold state, and explicit operations on those instances. This specification
explains how to read such programs, why the rules work together, and what a
conforming compiler, runtime, and library must provide.

The whole `spec/` directory is the source of truth for the Ama language and its
implementation requirements. Start here rather than treating one chapter or an
implementation checklist as the complete contract.

## Find your starting point

If you are learning the language, read [the introduction](introduction.md), then
[modules and state](language/modules-and-state.md). Continue with declarations,
values, and calls before exploring continuations or changing a list during a
traversal. The examples introduce the model before the detailed rules.

The [static-template chapter](language/templates.md) explains static inference
from typed connections, Type parameters introduced in inputs or results, named
constructor signatures, static application and partial composition. Constructor
patterns, named bindings, bidirectional type constraints and explicit constant
namespaces have agreed rules; their examples explain how those forms fit together.
The [constant section](language/declarations.md#const-06) records why parameterized
shared values and explicit namespace selection avoid name conflicts. The remaining
[open questions](../AMBIGUITIES.md#open-template-design-questions) identify
recursion, list-foundation, native/export and naming
boundaries. Agreed syntax is not evidence of compiler implementation.

If you are implementing Ama, read the language chapters first. Then use the
runtime, native-interface, and compiler chapters to see the required realization
of those rules. The [conformance chapter](implementation/tools-and-conformance.md)
describes the evidence needed to claim support.

If you need an exact source form, consult the [grammar](grammar.md). It describes
what can be parsed; the language chapters also determine whether a parsed
program is well typed and what happens when it executes.

## Language chapters

| Topic | What you will learn |
|---|---|
| [Introduction](introduction.md) | The purpose of Ama, its small core, and how to read an operation |
| [Modules and state](language/modules-and-state.md) | Types versus instances, ownership, interface state, and a complete example |
| [Source files, names, and layout](language/source-and-names.md) | Module paths, international names, exact spaces, comments, and sections |
| [Declarations and scope](language/declarations.md) | Explicit and inferred declarations, initialization, constants, and visibility |
| [Static module templates](language/templates.md) | Scoped interface Type parameters, typed-connection inference, named constructors, partial composition, specialization identity, dependent shared constants and explicit design gates |
| [Values and references](language/values-and-references.md) | Independent copies, checked references, identity, and resource lifetime |
| [Calls and results](language/calls-and-results.md) | Entry, resume, retained reads, method chains, and refusal handlers |
| [Control flow](language/control-flow.md) | Branches, condition loops, repeat boundaries, and early completion |
| [Continuations](language/continuations.md) | Yielding, resuming, and explicitly copying saved computation state |
| [Lists](language/lists.md) | Typed list methods, positions, live traversal boundaries, and mutation |
| [Asynchrony and parallelism](language/async-and-parallelism.md) | Explicit tasks, event delivery, cancellation, isolated execution, and messages |
| [Library contracts](language/library.md) | How native foundations and ordinary modules supply useful operations |
| [Grammar](grammar.md) | The maintained lexical and syntactic definitions |
| [Glossary](glossary.md) | Project terms and links to their detailed definitions |

## Implementation chapters

| Topic | What it specifies |
|---|---|
| [Runtime](implementation/runtime.md) | Stable pools, availability, control transfers, host outcomes, and execution contexts |
| [Native interface](implementation/native-interface.md) | C interfaces, the six native protocols, state boundaries, and lifecycle |
| [Compiled Ama libraries in C](implementation/c-library.md) | Exported C interfaces, static/shared packaging, checked host objects, and use inside another Ama native module |
| [Compiler](implementation/compiler.md) | Resolution, typed IR, generated C, component boundaries, and exact algorithms |
| [Diagnostics](implementation/diagnostics.md) | Expected refusals, fatal failures, status values, and execution traces |
| [Tools and conformance](implementation/tools-and-conformance.md) | Deliverables, tests, relocation tooling, and acceptance scenarios |
| [Implementation roadmap](implementation/plan.md) | An informative order of work, not an additional definition of the language |
| [Requirement index](requirements.md) | Stable requirement identifiers and their canonical chapters |

## How to read a rule

The language chapters define source-visible behavior. Implementation chapters add
required storage, ABI, architecture, tooling, and verification contracts; they
must preserve the language behavior. The grammar is the canonical definition of
source forms and follows those semantic contracts. A production can intentionally
recognize a form that a later static type or capability check rejects.

Requirement identifiers such as `COPY-06` and `FLOW-05` are stable references.
They help connect related behavior and acceptance evidence. They are not a
reading order, and a requirement can have several named subsections. Each
identifier has one canonical definition in this directory.

**Must**, **must not**, and **required** state acceptance requirements. **May**
identifies a permitted choice. **Should** identifies a preferred engineering
choice; replacing it requires a documented reason. Unmarked rule text in a
requirement section is also normative. A heading such as **Explanation**,
**Why this rule exists**, **Example**, or **Walkthrough** introduces informative
material: it explains the contract without adding exceptions or new syntax.
The introductory reading guides, glossary summaries, and implementation roadmap
are informative. Detailed rules govern if a summary omits a case.

Examples state whether they are complete module files or fragments with assumed
declarations. A hypothetical library path illustrates an interface unless the
rules explicitly fix that identity. In particular, the list interface does fix
`..integer` as its position/length type; that is a real library requirement.
An example is not evidence that the corresponding feature is implemented.

## Execution scope

An execution context has one Ama executor. Ordinary calls remain synchronous,
and `%` returns to the immediate caller. Programs can opt into the explicit task
and event-loop contracts in the asynchrony chapter. Independent contexts may
execute in parallel and communicate through explicit messages; mutable Ama
instances and checked references do not cross that boundary.

The [compiled-library contract](implementation/c-library.md) also permits a
C host, including an Ama native wrapper acting as that host, to call a separately
compiled component through its exported API. Each selected library context owns
its state independently. Data cross the boundary as declared independent native
values; an opaque foreign-context resource does not grant access to another
context's Ama instances.

No hidden scheduler changes the meaning of a normal call. There is no implicit
default instance, automatic retry of a busy target, general expression language,
or garbage collector. Native C foundations remain trusted implementations with
explicit contracts, not code that the Ama compiler proves correct.

## Maintaining the specification

Agents must obtain the user's explicit authorization for the particular change
before editing any file in `spec/`, including editorial changes, grammar,
examples, generated indexes, and the roadmap. A TODO item, implementation task,
refactoring request, or discovered ambiguity does not grant that authorization.
An explicit request to edit a specified part of the specification authorizes
that stated scope; it is not permission for unrelated changes.

If a task exposes ambiguous or contradictory required behavior, stop the
affected task before selecting an interpretation. Record the requirement IDs,
minimal example, competing interpretations, and precise question in the
root [ambiguity log](../AMBIGUITIES.md), then ask the user and wait for a decision.
The log records questions and answers; it does not amend the specification.
Implementation must not silently weaken a rule to fit existing code.

An authorized semantic change updates its canonical chapter, affected grammar,
examples, and acceptance scenarios together. Correct derived documentation if
it conflicts with this directory. Remove obsolete proposals and APIs rather
than preserving competing contracts. Preserve stable rule identifiers when
reorganizing prose; do not copy the same normative rule into another chapter.

The specification describes the target product. Implementation support must be
claimed only with acceptance evidence. Staging work in the roadmap neither
removes a requirement nor implies that a later-stage feature already works.

This edition reorganizes the design baseline of 2026-11-12 into a readable
specification and adds explicit asynchronous and isolated-parallel execution
contracts.
