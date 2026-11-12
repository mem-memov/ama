# Understanding Ama

[Specification index](README.md)

Ama programs are built from modules that keep their own state. Calling a module
means executing a particular instance. A result can itself be a module instance,
a reference, or a list, so a program can construct behavior and then invoke it
through the same ordinary call rules.

**Explanation.** Think of a module as a description of storage and behavior, and
of an instance as one concrete set of that storage. A declaration creates owned
state without running the declared child's body. A call acts on that existing
state. Ownership determines cleanup; references let other instances reach the
same state without taking ownership.

## A first reading example

**Example.** Assume project module `.counter` is callable and its first result is
an owned `value` of type `..integer`. This caller fragment owns one counter,
executes it, and then reads its retained result:

```text
@!
counter
.counter

counter !
 > value

counter
 value >> previous value
```

Read the program from top to bottom:

1. `counter` followed by `.counter` declares an owned field and its type. Its
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
| `.module` with indented `>>` | Copy shared constants from that namespace |
| `.module` / `..module` | Project/import module / standard-library module |
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
in examples are illustrative. `..integer` is a required nominal standard-library
identity with the exact capabilities specified for indices and list results;
it is not an interchangeable example type.

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
