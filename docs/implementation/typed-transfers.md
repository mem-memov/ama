# Exact transfers and lexical self

T032 implements static source typing under [TYPE-05](../../spec/language/declarations.md#type-05),
[REF-01](../../spec/language/values-and-references.md#ref-01) and
[COPY-06](../../spec/language/values-and-references.md#copy-06).
It does not certify the later runtime construction and clone-identity tasks.

## Transfer decision

A nonliteral source has a nominal type and two independent flags: reference and
list. Nominal identity includes the root kind and every path component. Matching
text in project and standard roots is insufficient. Ordinary transfers require
all three attributes to agree with the destination; there is no implicit
reference acquisition, dereference, scalar wrapping or element extraction.

| Source | Permitted ordinary destination |
| --- | --- |
| Owned scalar | Same-type owned scalar |
| Scalar reference | Same-type scalar reference |
| Owned-element list | Same-type owned-element list |
| Reference-element list | Same-type reference-element list |

Literal construction requires an owned scalar and its literal protocol. Explicit
continuation copying requires a scalar destination and the same nominal type;
it produces owned storage rather than retaining the source's reference flag.
Every owned copy requires static clone capability before any runtime alias/no-op
check. Reference transfers copy handles and do not acquire their targets.

[Source resolution](../../src/semantic/values/source.c) dispatches explicit source
categories to separate constant, named-access and self resolvers. The
[transfer checker](../../src/semantic/values/transfer.c) then applies the same
matrix to initialization, assignment and call bindings. Indexed accesses carry
the selected element's scalar form while retaining their complete index chain.
Qualified constants carry their resolved immutable declaration, not an instance.
Neither category changes the transfer matrix.

An unqualified shared source can have both a constant declaration and an access
chain. [Emission](../../src/codegen/emit/source.c) preserves that access chain;
only a qualified direct shared source without an access uses the shared-ordinal
shortcut. This distinction prevents `entries[position]` from becoming an access
to the entire shared list and preserves exactly one index conversion per access.

## Self means the containing module

The [self resolver](../../src/semantic/values/source/self.c) gives standalone `&`
the exact lexical module type, scalar-reference form and original one-byte site.
It has no named access, constant declaration or synthetic identifier. Destination
metadata never supplies its nominal type. Thus a default initializer belongs to
its containing declaration's module, whereas an explicit `< &` source belongs
to the caller that contains that argument. Shared initialization has no current
instance and rejects this source.

For example, a `.counter` result declared as `self&` of type `.counter` can have
initializer `&`. Declaring that result as `.other` fails; changing the result to
owned storage also fails. A reference-list initializer can contain scalar `&`
as one element, but assigning `&` to an entire reference list fails.

The proof is local: source resolution fixes `(type, reference, list)` before
transfer checking, and checking compares that tuple without mutating its type.
Only literals obtain their type from the destination. Self resolution is O(1)
with no allocation beyond the ordinary source record. Named and constant lookup
retain their existing finite lookup/access bounds.

## Executable boundary

Self sources now survive complete typed storage/control planning. The
[execution-capability guard](../../src/codegen/project/capabilities.c) checks every
initializer, assignment and binding before staging any C artifact. It reports
`unsupported-feature` at the original `&` until T049–T050 supply final runtime
construction and clone identity support. The plan retains a typed self source,
without a fabricated named access or a temporary replacement identity.

## Evidence

`make -C tests/unit/semantic/types` covers all sixteen form pairs with equal and
unequal nominal types, all sixteen cross-root form pairs, cross-root self,
missing/mismatched receiver suffixes, self in all initializer roles, individual
reference-list initialization, explicit caller inputs and fresh assignment
receivers. Every counted allocation in a real self-containing semantic pass is
failed independently and all owners must unwind.

`make -C tests/unit/compiler/check` proves that initializer, assignment and input
self sources reach complete valid IR. `make -C tests/unit/codegen/assembly`
verifies their execution-capability rejection before artifact staging. `make -C tests/unit/codegen/execution` compiles
and runs scalar and indexed shared reads; output traces check selected values,
independent call state and exactly one index conversion for each access.
