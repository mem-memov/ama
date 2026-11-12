# Values, ownership, references, and copying

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Forming and storing a reference](#forming-and-storing-a-reference)
- [Assignment and ordinary copying](#assignment-and-ordinary-copying)
- [Making a copy safely](#making-a-copy-safely)
- [Deleting storage without reviving old references](#deleting-storage-without-reviving-old-references)

<!-- /chapter-contents -->

An owned value is storage whose lifetime belongs to its owner. A reference is a
checked identity through which another operation can try to reach that storage.
Copying an owned value produces independent owned state. Copying a reference
preserves the identity of its target.

**Example.** Assume `original` is an owned `.shop.item` and `alias&` has the same
module type as a reference. These two assignments have different effects:

```text
independent
 original

alias&
 &original
```

If `independent` is a first-occurring receiver, TYPE-06 declares an owned field
and the first assignment copies `original` into it. The second assignment stores
a handle to `original`; it does not make another item. Changing `independent`
does not change `original`. Invoking `alias&` selects `original`, subject to its
liveness and availability.

**Why.** Explicit ownership permits deterministic cleanup, while checked handles
prevent an old reference from accidentally selecting a different object after
storage reuse. Neither reference assignment nor copying silently changes who
owns an object. [Continuations](continuations.md) adds an explicit copy form for
saved execution positions.

## Forming and storing a reference

### Named references and self-reference

<a id="ref-01"></a>

**REF-01.** `&item` obtains a checked reference to owned instance storage.
`target&` reads a reference variable. `&target&` is invalid: references to
references are not supported. The trailing ampersand remains part of reference
variable access, including named interface labels.

#### Referring to the current instance

Standalone `&` is a reference value source identifying the current instance of
the module containing that source expression. Its type is that module's exact
complete identity, including its root kind. It denotes neither the caller nor
an owning parent. Calling through a reference, selecting a result for execution,
or resuming a continuation does not change this rule: the called module's own
`&` identifies the instance actually executing its code.

The source is permitted wherever a compatible scalar reference source is
permitted: assignments, entry inputs, reference declaration/default initializers,
and individual reference-list initializers. It supplies a handle, never an owned
value or whole list. It has no identifier, introduces no declaration, and is not
a receiver, call/resume target, index, condition, or operand of continuation-copy
`%`. It is forbidden in shared `@=` initialization, which has no current instance.
Ordinary nominal compatibility still applies; `&` cannot acquire the destination's
type by inference or denote a field merely because it initializes that field.

For example, `.counter` may expose its own identity as an ordinary result:

```text
@>
self&
.counter
 &
@!
```

A caller can obtain it with `counter !` followed by an indented `> result&`.
The result uses the existing output transfer and TYPE-06 declaration rules;
there is no implicit additional result. The copied handle identifies `counter`,
and the caller may use it after the call releases its protections.

#### Which instance an initializer belongs to

Every initializer source is interpreted in the module whose declaration contains
it. Thus a callee's omitted parameter or result default `&` denotes that callee,
while an explicit input `< &` denotes the initiating caller under CALL-08/CALL-09.
Permanent-body initializers use their containing instance even though they run
before body execution. Creating an owned child's fields uses the child's own
declarations and instance context; initializing a field from `&` in its parent's
declaration does not switch to the field's context.

### What a checked handle identifies

<a id="ref-02"></a>

**REF-02.** Within one execution context, a reference identifies a module type, pool
slot, and slot generation.
The type may be statically known rather than stored in every handle. A reference
is not an exposed C address and does not keep its target alive. Generation zero
is reserved for empty references; zero-initialized handle fields represent an
empty handle, regardless of whether slot zero is otherwise valid. Live instances
use nonzero generations starting at one. An internal generic handle may carry
the type identity explicitly; a statically typed handle need not repeat it.

The triple is context-local. A handle formed in one execution context is never a
valid Ama reference in another, even if that other context has an equal type,
slot, and generation. Host wrappers that can address several contexts must carry
and validate context identity before touching a context-local handle. Message
serialization never transports an Ama handle as an identity to be used in the
receiver's context.


### Forming a handle does not acquire its target

<a id="ref-07"></a>

**REF-07.** Reference formation and runtime acquisition are different operations.
Prefix `&` forms a typed identity for existing owned scalar storage; it neither
locks that object beyond the current operation nor extends its lifetime.
Standalone `&` forms the containing instance's identity under the same rule,
including while that instance is running or waiting. Forming or copying this
handle does not acquire or reenter the target, save an execution position, or
retain a caller connection. A later operation through the handle still applies
all ordinary liveness, generation, and availability checks. In particular,
CALL-09 may refuse a direct operation on a self-reference result while its
producing instance remains protected by the enclosing operation tree.
Copying reference storage copies its identity, including an empty or stale handle,
without acquiring, cloning or reviving its target. Selecting an indexed reference
entry still checks the containing list and position. Target liveness, generation
and availability are checked when an operation resolves that handle for access.

## Assignment and ordinary copying

### Destination first, then source

<a id="ref-03"></a>

**REF-03.** Assignment places the receiver on one line and one compatible source
one indentation level below it. A first-occurring simple receiver declares
persistent storage under TYPE-06; otherwise the receiver must resolve to an
existing declaration:

```text
destination
 source

target&
 &item

other&
 target&
```

Compatibility and transfer effects are exactly TYPE-05's table. Resolve the
receiver location first, then the source, once each. An invalid standalone
selection is a technical failure; assignment has no attached `:`. Transferring
into reference storage changes the stored handle, not its target's state.

### Independent ownership and preserved external references

<a id="copy-01"></a>

**COPY-01.** Ordinary owned-value copying produces independent owned children
and lists. References stored inside the copy that point into the copied ownership
tree follow their corresponding new instances. References to objects outside that
tree retain their existing targets. Copying a reference variable alone copies
identity only and does not clone or redirect its target. A native resource that
cannot be cloned independently must report unsupported copying; it must not
silently become shared ownership.

### Value copying clears saved execution positions

<a id="copy-03"></a>

**COPY-03.** Ordinary owned copying copies data but no saved execution positions
or outstanding invocations anywhere in the copy boundary. The destination root
and all copied owned descendants, including owned list elements, have no resume
position. Replacing a distinct destination discards its previous continuation
state, including active foreach cursors and scoped binding cells. Escaped handles
stored in persistent fields still copy normally. Ordinary self-assignment
remains the no-op specified in COPY-06. Reference
assignment does not modify any referenced instance's execution state.

### The ownership boundary determines reference remapping

<a id="copy-04"></a>

**COPY-04.** The copy boundary is the source owned instance and all instances
recursively owned by it, including owned list elements. Shared `@=` constants
are outside every instance ownership/copy boundary. The source root itself
is included: an internal reference to that root points to the destination root
after copying. All checked references stored in the copied data, including
reference-list entries, use the same rule. The original objects and references
held outside the copy are never rewritten.

For example, if an object owns `item` and its `selected&` points to that item,
copying the object creates a new item and makes the copied `selected&` point to
that new item. Copying only `selected&` preserves its original target. No additional
copy mode or source syntax is introduced.

#### Copying a whole list or just one element

A whole owning-list copy treats all its owned elements and their descendants as
one copy boundary, so references between elements follow the new elements.
Copying one element alone does not include its siblings; references to those
siblings remain external. Copying a reference list alone does not own or clone
its targets and therefore preserves their identities.

#### Example: which references change in a whole-object copy

**Example.** Suppose source instance `original` owns a child `item`, has a
reference `selected&` to that child, a self-reference, and a reference to an
external object. Copying `original` into a distinct destination gives:

| Stored reference | Target before copying | Target in the copied value |
|---|---|---|
| `selected&` | Original owned child | Corresponding new owned child |
| Self-reference | `original` | Destination root |
| External reference | Object outside the source ownership tree | The same external object |

Only handles stored inside the copied value are adjusted. A handle held by some
other instance still points to the original child or original root. If only
`selected&` is copied, its target also stays the original child because no owned
child is part of that reference-only transfer.


## Making a copy safely

### An exact source-to-destination identity map

<a id="copy-05"></a>

**COPY-05.** Implement this as one operation-scoped clone context: record the
source-to-destination identity mapping for the complete ownership tree, then
resolve copied checked references against that mapping before publishing the
copy. This handles forward references, references to ancestors, and non-owning
reference cycles without recursively cloning referenced targets. Destination
replacement preserves the destination root's existing identity. Temporary mappings
and new children must be cleaned up on preparation failure, leaving the source
and previous destination unchanged. Only generated Ama storage contains persistent
Ama references; the compiler/runtime performs every corresponding reference fixup.
Native clone hooks copy their own native payload only and do not walk Ama handles.
Destruction errors after commit begins follow COPY-02 and PROTO-10.
The rule applies equally to ordinary value copying and `%` continuation copying;
execution-position handling remains governed by COPY-03 and CONT-04 through CONT-05.

#### Self-references follow the same mapping

A stored self-reference formed by standalone `&` follows these same rules: copying
the complete owned instance remaps it to the destination root; copying only its
reference result preserves the original target. Cloning does not reevaluate the
source expression or execute an initializer to recreate the handle.

### Capability checks, self-assignment, and overlap

<a id="copy-06"></a>

**COPY-06.** Every syntactic owned-copy operation must satisfy the static
copy-capability requirements of its source and destination types. This includes
assignments, initializers, and result transfers even when they will resolve to
the same runtime storage. A compiler must not exempt an operation by proving or
guessing that it is self-assignment. This rule gives one capability contract
regardless of aliasing or optimization.

After static validation and normal selection/access checks, an ordinary copy
whose source and destination are the exact same owned instance or collection
is a no-op. It does not clone, allocate, destroy resources, or clear a saved
execution position. Reference-variable self-assignment likewise leaves the
handle unchanged. Explicit `%` copying to the same instance is invalid: it
requires an independent destination. Identity checks use resolved runtime
identity, not merely equality of source spellings.

#### Ancestor/descendant overlap is forbidden

Copying owned state between an ancestor and its owned descendant is forbidden
in either direction, including ownership through lists. Distinct instances with
neither owning the other may be copied, including sibling elements of one list,
subject to normal type, availability, and lifecycle checks. Non-owning reference
links between source and destination do not themselves make ownership overlap.
Collection operations must also reject overlapping owned source/destination
boundaries; the self-assignment no-op is not permission to overwrite an ancestor
with its descendant.

#### When to check identity and overlap

Diagnose statically provable violations during compilation. Where identities or
ownership relationships are known only at runtime, validate them before changing
data, invoking clone hooks, or destroying destination resources. A runtime
violation is a technical copy failure under the explicit-status/diagnostic
protocol, not an assignment `:` alternative. Do not silently introduce temporary
snapshots to make forbidden overlap work. These rules apply to ordinary and `%`
copying and require no additional syntax or selectable copy modes.

#### Example: a no-op does not bypass the type contract

**Example.** Assume `.resource` can be constructed but does not support owned
copying. This module is rejected at compile time:

```text
@!
resource
.resource

resource
 resource
```

The assignment asks for an owned-copy operation of an unsupported type. There is
no special accepted spelling for the identical-name case. For a copyable type,
the same assignment passes static checks and its runtime identity check makes it
a no-op; no clone or destructor is invoked.


### The complete source tree must be available

<a id="copy-07"></a>

**COPY-07.** For both ordinary owned copying and `%` continuation copying,
validate availability across the complete source ownership tree before replacing
or modifying the destination. Refuse if any included instance is running or
waiting for a nested invocation. Keep the validated source protected for the
operation so native clone hooks cannot invalidate that check through reentry.
A self-assignment no-op does not initiate tree cloning. Destination access and
replacement remain subject to the existing acquisition/lifecycle rules.

#### References do not enlarge the copy boundary

The validation walks ownership edges only. External reference targets are not
cloned, need not be available for this copy, and do not contribute execution
positions or return links. Copying their handles still follows the normal checked
reference rules. No pending invocation chain is cloned, regardless of copy form.

### Preparation, commit, and failure

<a id="copy-02"></a>

**COPY-02.** Construction and clone failure before destructive replacement
preserves the source and old destination and releases all partial new resources.
After a complete replacement has been prepared, destruction of the old value
begins the nonrollback commit phase. A destruction error is terminal and cleanup
continues under PROTO-10; the partly destroyed destination is not restored or
exposed as live. Self-assignment and overlap follow COPY-06. Explicit continuation
copy requires distinct source and destination storage.

## Deleting storage without reviving old references

### Liveness and generations

<a id="ref-04"></a>

**REF-04.** Deletion makes the slot non-live immediately; before reuse its
generation advances without wraparound. Validation requires a nonempty handle,
the expected type, a live slot, and an exact generation match, followed by the
operation's availability check. Empty handles and stale handles never select an
instance. Deleting busy or acquired instances is forbidden. Destruction order
must release owned children exactly once. Reference cycles do not create
ownership cycles and do not prevent deletion.

### Generation exhaustion never wraps

<a id="ref-05"></a>

**REF-05.** Generation counters never wrap, reset, or reuse a previously issued
slot/generation identity. After deletion of an instance at the maximum generation,
permanently retire that slot instead of putting it back on the free list. Allocate
another slot if possible; identity-space exhaustion is an explicit technical
failure, not permission to wrap. Counter width is an internal ABI choice; tests
must exercise exhaustion with a reduced-width counter rather than rely on it
being practically unlikely. Retired slots must not become live again when a
block is reclaimed and later reallocated.

### Reclaiming blocks must retain identity history

<a id="ref-06"></a>

**REF-06.** Reclaiming payload blocks must preserve generation history and retired
slot information outside the freed storage, or permanently retire the released
slot identities and issue fresh ones. The invariant is absolute: an old handle
must never resolve to a different future object. Growth, shrinking, zero-filled
allocation, and free-list reconstruction cannot reset this history. Protection
covers Ama references within one execution-context lifetime; handles are not a persistence
format or permission to reuse references across destroyed runtime instances.

### Preflight all owned state before destruction

<a id="ref-08"></a>

**REF-08.** Before any normal destruction, removal, reset or replacement can
destroy owned state, preflight the entire affected ownership subtree, not only
its root. No included running, waiting or incompatibly acquired descendant may
be destroyed. Protect the validated tree through the operation. The operation's
own authorized acquisition is not a conflicting acquisition. Non-owning reference
edges do not extend the destruction boundary. Clear preflights all its owned
element subtrees before any destruction. Invocation target refusal follows
CALL-04; list-method preflight refusal follows LIST-10; other dynamic access violations
are technical failures. Cleanup after execution has already failed first releases
active links/acquisitions under LIFE-05, then destroys owned state.
