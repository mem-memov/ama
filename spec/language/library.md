# The standard library and native foundations

[Specification index](../README.md)

## Explanation: the library supplies operations, the language composes them

Ama has no built-in arithmetic expression syntax or universal numeric payload.
A module can interpret literal bytes as an integer, text, or another value;
another module can operate on that type through declared inputs and results.
The compiler checks how those contracts are connected.

This makes the public library interface important. A reader should be able to
discover an operation's accepted values, effects, results, and failures from its
module contract. Calling a library module does not change the parsing of Ama.

The intrinsic list interface is an explicit exception to ordinary module-valued
methods: its six entries are part of the collection contract. Their English
interface names are not additional grammar keywords. `..integer` is the fixed
position/length type for that interface. Its native contract defines its binary
representation; generated list code uses the declared protocols and
[PROTO-03's typed host-size bridge](../implementation/native-interface.md#proto-03)
without inspecting that representation.

## Required library surface

The library implementation must provide the following agreed capabilities.
Missing capabilities are implementation gaps, not alternative language rules:

- Integer/index types with literal decoding, ranges, arithmetic, comparison,
  overflow behavior, and checked conversion to host allocation/index sizes.
- A truth-capable type and a registered condition conversion contract.
- Text/byte-sequence types with explicit byte lengths and ownership; their native
  contracts may use COPY-01/PROTO-06's shared immutable backing while preserving
  independent values and lifetimes.
- Dynamic owned/reference list operations matching the chosen collection contract.
- Resource lifecycle declarations usable by native modules.
- Allocation and I/O implementations consistent with the selected stackless boundary.
- An explicit task/event protocol and isolated-context message transport under
  [asynchrony and parallelism](async-and-parallelism.md), with documented public
  types and interfaces rather than hidden changes to invocation.

## Validated native foundations

<a id="lib-01"></a>

**LIB-01.** Compiler mappings identify module operations through validated
signatures and identities, not hard-coded assumptions about a numeric payload.
Native condition/index mappings use the explicit symbolic `.abi` bindings in
PROTO-01 through PROTO-10, with `=` declaring native copy capability. Public module paths must be documented and tested.
The fixed `..integer` host-size constructor in PROTO-03 is an additional typed
library helper used for LIST-04 length results, not a seventh symbolic binding
or an implicit numeric conversion for other module types.
Moving integers into the library must not hide an equivalent privileged integer
in literal syntax, condition evaluation, or list indexing.

<a id="lib-02"></a>

**LIB-02.** Libraries may place shared constants beside their module behavior in
`@=` under CONST-01 through CONST-05. Built-in list operations instead use
LIST-04's typed method interface and never a constant selecting an action.
Module invocation still executes
explicit behavior; shared constants do not imply automatic reactive updates.

## Explanation: document a native operation as a complete contract

A useful operation description answers the same questions as an Ama call:
what inputs it accepts, what state it reads or changes, what it returns, and
what happens when it cannot finish. For a resource, also state who owns it,
whether it can be cloned, and how cleanup works. For I/O, distinguish a normal
application outcome such as end-of-stream from a fatal technical failure.

An asynchronous operation additionally describes its registration, possible
late completion, cancellation, and buffer lifetime. A parallel worker describes
the message schema and thread-safety obligations of its native dependencies.
These details belong to the library contract; they are not new Ama operators.
