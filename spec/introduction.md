# Understanding Ama

[Specification index](README.md)

Ama programs execute ordinary modules that keep their own state. Calling a module
means executing a particular instance. A result can itself be a module instance,
a reference, or a list, so a program can construct behavior and then invoke it
through the same ordinary call rules.

Static modules, marked `@...`, instead connect types and templates during
compilation. Their purpose is to generate the concrete modules a program needs
without making its author write each combination by hand. They have no runtime
instance or mutable state; their results provide types for ordinary declarations. See
[static modules](language/static-results.md#tpl-12) after learning ordinary calls.

**Explanation.** Think of a module as a description of storage and behavior, and
of an instance as one concrete set of that storage. A declaration creates owned
state without running the declared child's body. A call acts on that existing
state. Ownership determines cleanup; references let other instances reach the
same state without taking ownership.

## A first reading example

**Example.** Assume project module `/example/demo/v1.0.0/src/counter` is callable and its first result is
an owned `value` of type `//integer`. This caller fragment owns one counter,
executes it, and then reads its retained result:

```text
@!
counter
/example/demo/v1.0.0/src/counter

counter !
 > value

counter
 value >> previous value
```

Read the program from top to bottom:

1. `counter` followed by `/example/demo/v1.0.0/src/counter` declares an owned field and its type. Its
   storage is created with the enclosing instance. This declaration does not
   execute the counter.
2. `counter !` starts that particular counter at entry. The indented `> value`
   receives its first result. Because the caller's `value` first appears as a
   receiver, TYPE-06 declares a persistent owned field with the result's type.
3. The unmarked `counter` with `value >> previous value` reads the named stored
   result without executing the counter again. The left `value` names the
   counter's result; `previous value` names the caller's receiver. It is another
   inferred persistent field.

After both operations, `value` and `previous value` contain independent copies
of the same result. A later call can change the counter's retained result without
changing either earlier copy. The spaces in `previous value` are part of one
name. One leading space expresses one level of nesting.

The complete module and instance model is explained in
[Modules and state](language/modules-and-state.md). The exact parsing rules are in
[Grammar](grammar.md).

## The decisions made visible in source

Ama asks a program to state ownership, execution mode, and transfer direction at
the place where they matter. Those visible decisions support predictable reuse
of state and explicit resource lifetimes.

| Source form | Read it as |
|---|---|
| `item` / `target&` / `items[]` / `targets&[]` | Owned value / reference / owning list / reference list |
| `&item` in source position | Form a checked non-owning reference |
| Standalone `&` in source position | Refer to this containing instance |
| `destination` followed by indented `source` | Transfer into the destination; a first receiver can declare its field |
| `worker !` | Start the existing instance at entry and prepare its interface |
| `worker %` | Resume its saved position without resetting the interface |
| `worker` with indented `>>` | Copy retained results without executing the target |
| `worker` with indented `method !` | Call a named returned module in its existing storage |
| `items[item&]` followed by `{` | Visit list entries by reference using a live boundary |
| `/example/demo/v1.0.0/src/module` with indented `>>` | Copy shared constants from that namespace |
| `/example/demo/v1.0.0/src/module` / `//module` | Project/import module / standard-library module |
| Standalone `%` / prefix `% source` | Yield to the immediate caller / copy a continuation |
| Condition followed by `?`, `:`, or `{` | Positive branch, negative branch, or condition-controlled loop |
| `:` / `^` / standalone `>` | Negative branch or refusal alternative / exit the enclosing loop / finish this invocation |

An instance-operation chain ending in a condition followed by `{` recomputes the
whole chain for each condition check (FLOW-04). Direct selected-result chains
follow CALL-09. These are explained with examples in
[Calls and results](language/calls-and-results.md) and
[Control flow](language/control-flow.md).

Execution suffixes `!` and `%` announce that a target may run and change its state.
A retained read does not run its target, although copying its results changes
the receiving storage. Symbols in literal payloads after `|` are data. Names,
paths, and markers have one interpretation in each grammatical position; their
meaning is never chosen by inspecting a C implementation.

## Why the core stays small

<a id="core-01"></a>

**CORE-01.** Ama aims for a minimal but powerful, coherent language core, not a
sequence of deliberately incomplete language versions. Implementing the agreed
language in stages is an engineering plan, not a promise to keep extending its
syntax or observable semantics after each stage.

<a id="core-02"></a>

**CORE-02.** New domain capabilities belong in libraries built from modules,
instances, references, collections, and native C interfaces. Prefer library
composition over new keywords, operators, special built-in types, or context-specific
syntax. The documented condition/index mappings are explicit core contracts,
not permission for libraries to redefine parsing or silently alter core behavior.

<a id="core-03"></a>

**CORE-03.** Resolve remaining design questions as parts of one consistent core.
Do not justify an arbitrary restriction by assuming a later syntax expansion.
A core change requires an independently demonstrated need that the existing
library mechanisms cannot adequately express, with grammar, semantics, diagnostics,
and tests reviewed together. This does not prohibit correcting specification bugs.

**Explanation.** New libraries can supply data representations, operations,
event loops, and domain interfaces. They compose through the same module,
ownership, call, and reference rules. A programmer should not need a new parser
extension to use a new library.

## Native foundations and their limits

<a id="core-04"></a>

**CORE-04.** Native modules provide the trusted foundations of Ama. Treat each
native operation as a contract: typed inputs and outputs, preconditions,
postconditions, permitted state/resource effects, and failure behavior. Ama adds
qualified identities, typed composition, explicit ownership and control flow
around those operations. Library growth adds operations and types within this
contract; it does not add grammar productions or alter existing operators.

The analogy with mathematical proofs is compositional: if each primitive meets
its contract and each composition rule preserves the required invariants, the
composition satisfies those invariants. A native declaration is an assumption
about C, not a proof of its implementation. The Ama compiler checks declarations,
types, capabilities and generated uses; it does not prove arbitrary C free of
undefined behavior, resource leaks or nontermination. Native implementation and
toolchain obligations must remain explicit in every correctness argument.

<a id="core-05"></a>

**CORE-05.** Ama's core fixes naming, storage forms, lifecycle, typed transfer,
invocation, control flow and the six native protocols. A library may interpret
literal bytes and implement domain behavior, but cannot change indentation,
parsing, assignment, reference validity, acquisition, or error propagation.
There is no open-ended operator or parser registry, implicit conversion, general expression
language, or execution selected by searching for a conveniently named C function.

The list interface is one fixed core contract: its six members are `append`,
`insert`, `remove`, `clear`, `swap`, and `length`. These are intrinsic interface
names, not source-language keywords or library-defined parsing rules. Most paths
in examples are illustrative. `//integer` is a required nominal standard-library
identity with the exact capabilities specified for indices and list results;
it is not an interchangeable example type.

## Generation and confidence in behavior

**Explanation.** A sorting module should be reusable for different element and
comparison modules. Static composition supplies each required concrete
specialization. It does not establish that the comparison orders elements
consistently or that sorting preserves every input element. Those are behavioral
contracts, checked by the module's tests and the tests of its real compositions.

<a id="core-06"></a>

**CORE-06.** Static modules construct and compose types and templates so the
compiler can generate concrete ordinary modules from their definitions. They
remove repetitive specialization work from the programmer. Each used executable
specialization must be fully concrete and satisfy the ordinary language checks
before code generation, under [TPL-06](language/templates.md#tpl-06). Static
composition follows the structural equations and finite-generation rules in
[TPL-10](language/templates.md#tpl-10) and
[TPL-11](language/templates.md#tpl-11); it does not execute arbitrary Ama bodies
during compilation.

The language does not add runtime type queries, dispatch on type metadata,
dependent types or a proof language to establish domain-specific
behavior. Express observable domain distinctions through ordinary module
interfaces and values. Specify behavioral properties in module contracts, and
require test evidence for those contracts instead of encoding such properties
as additional type-level logic. Tests must cover representative examples,
properties, boundaries, relevant invalid inputs and operation sequences; compare
against an independent reference implementation when one is applicable. A finite
set of successful tests establishes evidence for the covered cases, not a
universal proof over untested inputs. Exhaustive claims must state the finite
domain actually checked.

This separation retains the compiler's required checks of type compatibility,
layout, ownership, reference validity, lifecycle, operation capabilities and
finite specialization. Passing tests never admits a program that fails those
checks, changes its inferred types or selects its generated implementation.
Conversely, passing the compiler does not establish a behavioral contract:
tests must reject a faulty implementation even when it is well typed. The
implementation's general correctness arguments remain required by
[ARCH-05](implementation/compiler.md#arch-05) and
[ARCH-06](implementation/compiler.md#arch-06), alongside the tests.

**Examples.** For sorting, test ordering and preservation of all input
occurrences, including duplicates. For a hash-based collection, test that equal
keys have equal hashes and that lookup survives the documented updates. For a
module that uses an enumeration to identify the active alternative, test that
its tag agrees with its stored payload and that invalid operations produce the
documented failure. These properties are checked through ordinary inputs,
outputs and effects; they do not require inspecting a type at runtime.

## How to continue reading

Read [Declarations](language/declarations.md) to distinguish construction,
execution, and visibility; then read [Values and references](language/values-and-references.md)
for ownership and copying. [Calls](language/calls-and-results.md),
[Control flow](language/control-flow.md), and [Lists](language/lists.md) show how
these rules compose. [Continuations](language/continuations.md) explains explicit
suspension. [Asynchrony and parallelism](language/async-and-parallelism.md) builds
task scheduling and isolated parallel execution from those contracts.

Requirement identifiers remain stable reference points. Motivation and examples
are labelled as explanation; exact behavior is specified in the associated
requirements and algorithms. The [specification index](README.md) states the
authority of these documents and separates language contracts from compiler
implementation and conformance requirements.
