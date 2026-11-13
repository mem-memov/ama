# Acquisition and destruction preflight

T048 verifies the existing protection implementation under
[REF-08](../../spec/language/values-and-references.md#ref-08),
[COPY-07](../../spec/language/values-and-references.md#copy-07),
[CALL-04](../../spec/language/calls-and-results.md#call-04),
[CALL-08](../../spec/language/calls-and-results.md#call-08) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
It composes [stable storage](instance-ownership.md) with real object and invocation
consumers. Typed selected-call/list-service execution adapters remain later tasks.

## Exact boundary and authority

[Tree preflight](../../src/runtime/objects/access/tree.c) follows initialized
ownership edges only. The root and every included child must have a valid nominal
identity, the expected parent and available service state. Reference fields and
reference-list elements do not extend that boundary: an unrelated running target
is not implicitly acquired or destroyed. Copy operations also check declared
clone capability, including element types of empty owning lists.

Every protected identity has an operation-owned record capturing its complete
previous service state. The operation owns that restoration authority and keeps
the record arena alive through the protected work. A nested copy may borrow the
outer operation's explicit copy-authority list. Its own records then do not own
restoration; releasing them cannot release the outer protection. An unrelated
operation cannot infer authority from an acquired activity flag. Borrowed copy
authority does not authorize destructive preflight. Only exact handle identity
matches; no nominal-only or address-only match grants permission.

On allocation, identity, ownership or availability failure, the initialized prefix
of records remains owned by the operation. Unprotect restores each record that
owns restoration and still refers to the same occupied generation. Arena release
comes afterwards. This prevents a failed traversal from losing saved positions,
return connections, refusal history or earlier operation identifiers. Cleanup and
replacement explicitly withdraw restoration where the old state no longer exists.

For a valid finite ownership tree, each visited node either produces one complete
record or stops with failure. Induction over the initialized child prefix proves
that every successfully visited descendant is protected before any destructive
hook can run. The public operation guard prevents reentry during the synchronous
preflight/effect/cleanup interval. Source-authority borrowing preserves the outer
owner's protection until its own end. The set of restoration owners is therefore
unchanged by nested read-only borrowers. Allocation failure can shorten the
visited prefix, but cannot create an unowned acquired record.

Traversal uses explicit records and child metadata rather than recursive C calls.
For V visited instances and E declared elements, descriptor/child scans and exact
record searches give the conservative tree bound O((V+E)^2 + V*T), where T is the
number of registered types. Copy-capability checking is a separate fixed-point
scan: with F registered field descriptors, at most T passes each perform type
scans and linear nominal lookups, bounded by O(T^2*(F+1)). Add that bound for a
copy preflight. Scratch is O(V+T). No constant-time or linear-time claim is made
for whole-tree preflight.

## Invocation and host boundaries

Invocation selects and checks the target before acquiring it. A supported target
refusal records the immediate caller's snapshot and explicit refusal provenance.
A successful acquisition records the target's previous activity on the caller;
release restores it and clears the outgoing connection. Entry preparation checks
all interface-owned subtrees before replacement and protects supplied owned
sources while fields commit in declaration order. Reads and resumes skip entry
initialization. A later preparation failure releases its protections/acquisition;
earlier successful field commits remain, according to the replacement contract.
Preparation failure is not reclassified as target refusal merely because its
numeric status equals a refusal code. Permanent body fields are not part of an
interface reset unless actually affected by the operation.

Host-facing [context resolution](../../src/runtime/context/references.c) checks
origin before interpreting the local type/slot/generation triple. Two contexts may
legitimately issue equal local triples; a foreign token still refuses before
reading the receiving context's object. Forming a checked reference to the current
running instance does not grant permission to acquire it.

## Executable evidence

Run `make -C tests/unit/runtime` for the composition boundary. Independent leaves
supply these observable checks:

| Leaf | Evidence |
| --- | --- |
| `objects/preflight` | Nine owned nodes with distinct Ready/Yielded metadata and an unrelated Running referent; explicit nested copy authority; outsider/destructive refusal; outer lease retained after nested release; every capability/record allocation failed; invalid parent, duplicate ownership and wrong child type; exact full-service restoration and no native effects. |
| `objects/access` | Every node of a nine-node source/destination tree blocked by Running, Waiting or Acquired state before hooks/destruction; copy overlap; native reentry; deletion allocation failures; authorized nested initializer reads. |
| `calls` | Busy inputs, first target-refusal snapshot, read/resume behavior, literal/constructor failure and every preparation allocation failure releasing acquisitions; successful earlier commits retained. |
| `call-preparation` | Source selection once in textual order; repeated source leases survive distinct commits; list/container overlap rejected before construction. |
| `contexts` | Colliding local triples from separate contexts rejected by origin; running-reference formation kept distinct from acquisition; independent context state and admission. |

The new direct leaf compares each logical service field explicitly, avoiding C
padding comparisons. Native constructor/clone/destructor counts and counted
allocations establish that preflight has no native effect and teardown releases
its complete prefix. Strict runtime composition, address/undefined-behavior
sanitizers, standalone-header checks and reproducible Doxygen generation accompany
the [per-file coverage registry](documentation/ready.json).
