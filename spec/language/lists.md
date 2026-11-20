# Working with lists

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Owning lists and lists of references](#owning-lists-and-lists-of-references)
- [Selecting an element by its current position](#selecting-an-element-by-its-current-position)
- [Calling list methods](#calling-list-methods)
- [Examples: adding, removing, and observing entries](#examples-adding-removing-and-observing-entries)
- [Traversing a changing list](#traversing-a-changing-list)
- [Access protection and safe failure](#access-protection-and-safe-failure)
- [Length and whole-list copies](#length-and-whole-list-copies)
- [Membership identity and traversal bookkeeping](#membership-identity-and-traversal-bookkeeping)
- [Required representation and complexity](#required-representation-and-complexity)
- [Explanation: templates and the list implementation](#explanation-templates-and-the-list-implementation)

<!-- /chapter-contents -->

A list keeps values in a definite order and can grow or shrink. An owning list
owns each element's lifetime; a reference list records access to values owned
elsewhere. Operations use named methods, so their inputs and results can be
checked before execution.

The first sections explain indexed access and the six methods. The live-traversal
section explains what happens when a loop changes its own list. The final
sections define the exact protection, identity, and storage contracts required
to implement that behavior.

**Why entries have identities.** A reference list may contain the same target
twice. Removing the first entry removes one occurrence, even if the target still
exists through the second. A membership identifies that particular occurrence;
a target handle identifies the value to which it refers.

## Owning lists and lists of references

<a id="list-01"></a>

**LIST-01.** Lists are language-level dynamically sized typed collections.
`items[]` owns its elements; `targets&[]` stores checked references. List storage
is distinct from the runtime pool of all instances of its element type. Every
list has its own checked storage identity, order, capacity, membership records,
and acquisition state. Its identity comprises the containing instance's typed
slot/generation, a fixed compiler-known field selector, and a collection
incarnation under LIST-13. Neither an empty list nor a list of references is an
absent list.

## Selecting an element by its current position

<a id="list-02"></a>

**LIST-02.** Indexed access uses a named index: `items[position]` or
`targets&[position]`. Brackets select storage without executing its target;
`items[position] !` executes the selected instance, and `%` resumes that instance
under the ordinary continuation rules. A missing position, empty/stale reference
target, or unavailable selected target refuses before its body or argument
defaults execute. No operation on an element may silently select another
position or run against removed storage.

Positions are zero-based. Access, replacement, and removal require
`0 <= position < n`; insertion permits `0 <= position <= n`, where `n` is the
current length. Inserting at `n` appends. An empty list permits insertion at zero
and has no accessible/removable position. Protocol `[]` supplies the position
directly, without an implicit offset or subtraction. Validate conversion and
bounds before using an identifier-array position.

The distinct `items[item&]` / `targets&[item&]` header before `{` binds a foreach
reference under FLOW-05. A reference name inside those brackets is not an index.
Whole-list service access uses a bare `items[]` or `targets&[]` parent followed
by selected methods under LIST-04. Whole-list `!` and `%` are invalid: a list
does not execute all its elements, dispatch an action value, or have an execution
position of its own.

## Calling list methods

<a id="list-04"></a>

**LIST-04.** Every list exposes the following compiler-provided typed service
interface. The member names in this table identify interface entries, not action
constants or language keywords. All six methods execute with `!`; the parent
list has no input/output bindings of its own and must contain at least one
selected method action. Normal named/positional binding, omission, placeholders,
and receiver-inferred declarations apply separately to each method.

| Method | Inputs, in declaration order | Results, in declaration order |
|---|---|---|
| `append` | `source&`: element reference, default empty | `element&`: reference to the appended target |
| `insert` | `position`: `//integer`, default zero; `source&`: element reference, default empty | `element&`: reference to the inserted target |
| `remove` | `position`: `//integer`, default zero | None |
| `clear` | None | None |
| `swap` | `first_position`: `//integer`, default zero; `second_position`: `//integer`, default zero | None |
| `length` | None | `value`: owned `//integer` containing current length |

The reference type is specialized to the list's exact element module identity,
including root kind. The standard-library module `//integer` must provide
ordinary copying, a literal initializer for every nonnegative position/length
representable by the implementation, a read-only `[]` conversion covering
that full supported range, and the exact typed native `from_size` constructor
defined by [PROTO-03](../implementation/native-interface.md#proto-03). Omitted
position inputs use its `|0` initializer; length results use `from_size`,
including zero. Its public nominal module identity is fixed here; runtime code
must still use the declared native protocols, not assume a numeric payload
layout. ERROR-05/PROTO-05's reserved invalid-index conversion status, including
negative/host-size-overflow positions, is an expected LIST-10 preflight refusal,
as is a successfully converted position outside current bounds. Other native
conversion failures remain technical.

These methods are intrinsic service entries lowered to typed compiler IR, not
hidden owned module instances or ordinary first-class module-valued results.
They cannot be copied through `>`/`>>`, referenced with `&`, resumed with `%`,
or executed through a type path. Their output values, including `element&`, are
ordinary typed values and may be copied or used in further CALL-09 operations.
No method keeps an implicit action selector, last completion flag, outcome
enumeration, or retained-result state. Call `length !` whenever a current length
is required; copying that result does not create a reactive view of the list.

After successful preflight, materialize each method's declared result in typed
compiler-visible service-frame storage. An owned result needing instance
identity uses an ordinary stable pool slot; it is not a hidden method instance
or a C-stack activation. Keep results live and protected through their complete
output-action subtree, then finalize them exactly once before the next sibling
or failure cleanup. Ordinary copies into caller fields survive this cleanup;
escaped non-owning references to an ephemeral owned result become stale.
`length` constructs `value` exactly once from the stored `size_t` length through
`//integer`'s explicit typed native `from_size` constructor. Initialize its
declared Ama fields normally, then select that constructor for its fresh native
payload under [LIFE-02](../implementation/native-interface.md#life-02); do not
first construct a payload with `+` or `|` and then overwrite it. The bridge
represents every value from zero through `SIZE_MAX` exactly. Its technical
failure uses the normal status/partial-construction cleanup path, never a `:`
range refusal. Result protection and destruction remain unchanged.

**Why use a typed bridge.** List length is already structural machine metadata.
Formatting it into decimal bytes and reparsing those bytes would add work to
every `length !` without changing the resulting integer. The generated code
instead includes the fixed `//integer` native header in its qualified context
and calls the declared constructor with the current length. This neither
assumes the integer payload layout nor introduces a numeric source literal,
untyped machine-value source, or seventh `.abi` protocol. Source literals still
use their ordinary `|` protocol. Acceptance checks cover empty lists, lengths
around native integer representation boundaries, the full bridge range, and
constructor failure/cleanup; a test hook must confirm that length construction
does not invoke decimal formatting or literal parsing.

### What each method changes

<a id="list-08"></a>

**LIST-08.** `append` creates one entry at the end. `insert` creates one entry
before its position. `remove` destroys/removes exactly the selected entry;
`clear` removes all entries. `swap` exchanges two membership identifiers without
cloning targets or changing which memberships exist. Swapping a position with
itself succeeds without a mutation, as does clearing an empty list. `length`
returns current length without executing element bodies or changing membership.

Methods accept only their declared arguments. There are no irrelevant parameter
positions whose values must be supplied or ignored according to a runtime
action value. A parent may select several methods in source order using
CALL-09; each finishes its effects and complete result-action subtree before
the next method begins.

### Where new entries get their values

<a id="list-09"></a>

**LIST-09.** Owning-list `append`/`insert` constructs a new stable-pool instance.
An omitted/placeholder source uses the element type's initial state without
executing its body. An explicitly supplied empty source reference requests the
same initial state. A supplied nonempty source requests an independent ordinary
clone: recursively clear saved positions and remap internal references under
the copy rules. Source ownership is never transferred.

Supplying `source&` in an owning-list method statically requires clone capability
for the element type, even if that handle is empty at runtime. Omission or an
empty positional placeholder does not require clone capability. Do not infer
capability from runtime emptiness. This check belongs only to these two
copy-producing interfaces, not to `remove`, `clear`, `swap`, or `length`.

Reference-list `append`/`insert` stores the complete supplied handle, including
an empty or stale handle, without cloning, creating, or acquiring the target;
an omitted source stores an empty handle. Both list kinds return the appended
or inserted target handle as `element&`. For a reference list this is exactly
the stored handle, so it can be empty or stale. Foreach skips empty/stale
reference entries under LIST-13/FLOW-05 rather than executing nonexistent targets.

## Examples: adding, removing, and observing entries

<a id="list-12"></a>

**LIST-12.** These examples assume declared compatible `sample`, `position`, and
`second_position` variables. `new_item&` and `count` can use TYPE-06 inference.
`items[]` is an owning list of the same module type as `sample`.

```text
# Append a default-created element, then append an independent copy.
items[]
 append !
  > new_item&
 append !
  < &sample

# Insert a copy before the current named position.
items[]
 insert !
  < position
  < &sample

# Remove an entry; handle a missing position without executing a deletion.
items[]
 remove !
  < position
 :

# Execute two more methods under one service acquisition.
items[]
 swap !
  < position
  < second_position
 length !
  > count
```

The first service appends two independent instances: one in its initial state
and one copied from `sample`. `new_item&` refers to the first instance. The last
service swaps two positions and then measures the resulting list under the same
acquisition. Its copied `count` remains an ordinary integer after the service
ends; later edits to the list do not change it.

The deliberately empty refusal handler after `remove !` discards a refused
removal and produces the ordinary empty-body warning. It changes no entry and
does not retry. To perform recovery work, put its statements one level beneath
that `:`.

The following owning-list traversal removes the current first entry on each
iteration. Deletion shifts its successor to zero, and the boundary correction
ensures that the successor is considered next, until the list is empty. The
body does not use the removed iterator handle afterward.

```text
zero
//integer
 |0

items[item&]
{
 items[]
  remove !
   < zero
}
```

The binding is a handle, not a permanent position. With current `B` selected,
`[A, B | C, D]` has boundary `q = 2`. Independent mutations have these effects:

| Mutation | Order and corrected boundary | Subsequent candidates |
|---|---|---|
| Insert `X` before `B` | `[A, X, B | C, D]` | `C, D` |
| Insert `X` immediately after `B` | `[A, B | X, C, D]` | `X, C, D` |
| Append `X` | `[A, B | C, D, X]` | `C, D, X` |
| Remove `A` | `[B | C, D]` | `C, D` |
| Remove `B` | `[A | C, D]` | `C, D` |
| Remove `C` | `[A, B | D]` | `D` |
| Remove `B`, then insert `X` at its former position | `[A | X, C, D]` | `X, C, D` |
| Clear, then append `X` before the body ends | `[ | X]` | `X` |
| Swap `A` and `C` | `[C, B | A, D]` | `A, D`; `A` repeats and `C` is excluded |

These are membership labels, not source syntax; every listed candidate is
subject to the ordinary reference-target eligibility check. For a reference list,
atomically replacing `B`'s handle creates a new membership behind the unchanged
boundary, so that replacement is not visited again by this pass. A body that
unconditionally appends one eligible entry per iteration can run forever. The
body may use separate list-service statements, branches, or nested loops. A
deletion targeting a busy owned instance still refuses under LIST-06/LIST-10.

## Traversing a changing list

<a id="flow-05"></a>

**FLOW-05.** A foreach statement scans an owning or reference list by scalar
reference using a live boundary in its current zero-based order:

```text
items[item&]
{
 item& !
}

targets&[target&]
{
 target& !
}
```

These are alternative body fragments with declared compatible source lists.
Each selected reference denotes the actual element, so the call acts on that
instance rather than an independent copy. Adding or removing entries in the
body follows the boundary rules below. No element is kept busy merely because
the loop's binding refers to it.

The source is one mutable list field in the current instance: `items` must
resolve to an owning-list declaration and `targets&` to a reference-list
declaration. A constant, type path, scalar reference to a module, indexed
expression, or direct result selector is not a foreach collection source.
The binding inside the brackets is exactly a simple reference name, including
its adjoining `&`; multiword names retain their single internal spaces. No
spaces surround the brackets or either ampersand. The header, `{`, and `}`
align; the body is one space deeper. Only comments at the header's level may
separate it from `{`, not blank lines. The ordinary body/comment layout and
empty-body warning apply.

Recognize this statement structurally before name resolution. `items[index]`
followed by `{` retains FLOW-02's indexed-condition meaning; `items[item&]`
followed by `{` is always foreach, never an index or truth test chosen by the
existence/type of `item`. Foreach is an ordinary statement, not a terminal
condition or link in FLOW-04's condition chain. Its body may contain nested
foreach statements, conditions, calls, and ordinary loops.

### Binding and scope

Resolve binding scopes by the following exact static algorithm:

1. Collect all explicit module declarations, including declarations written in
   ordinary nested bodies. Keep TYPE-06's earlier inferred fields as the
   source-order traversal records them; never precollect later inferred fields.
2. Traverse the parsed executable bodies in source order, maintaining the active
   enclosing foreach-local bindings. Resolve the collection in the incoming
   environment before considering its new binding. Its declaration identity,
   list storage form and element type term must come from an explicit declaration
   or earlier inferred list field. The term need not yet be concrete.
3. For the binding's complete base name, look for an enclosing local binding,
   then an explicit module declaration or earlier inferred persistent field.
   If present, reuse that variable and require a mutable scalar reference of
   exactly the list's full element-module type. Reject a constant or incompatible
   storage form; retain a type-compatibility equation when its concrete type is
   not yet known. A later type mismatch is an error, not permission to create
   another binding, shadow the name or convert its storage.
4. If absent, declare a scalar reference local to this foreach body with the
   list's exact element type term. Make it visible throughout that body and nested
   executable bodies, but not in its own collection source, following code, or
   module construction. Pop that binding when leaving its lexical body.
5. Classify body uses against this environment. New local binding declarations
   and uses resolved to them do not enter TYPE-06's module-name occurrence set.
   Assignments and output bindings to a local reference update that local cell.
   Other new assignment/output receivers still declare persistent module fields
   under TYPE-06. A later field inferred outside the local binding's scope may
   reuse its base name and cannot retroactively change that earlier binding.

Explicit declarations anywhere in the module retain their existing visibility;
an explicit compatible reference declared later is therefore reused, rather
than causing an earlier foreach to introduce a local reference. Explicit and
receiver-inferred fields inside the body remain permanent fields. Their
creation-time initializers cannot read a runtime foreach-local binding; an
executable assignment may copy its handle into a permanent field or result.
The body's entire syntax, names, types and capabilities are checked even when
the source will be empty. No runtime list contents participate in scope or type
inference. Name-dependency edges from an inferred body field through a local
binding lead to that earlier header and its list declaration. This preserves
TYPE-06's lexical declaration-position argument; type equations are a separate
graph and may be refined by later typed connections. Every required type is
concrete before owner layout/construction or runtime execution. Deferred type
checking changes no name scope and creates no runtime binding before the list
validation and eligible-element activation specified below.

### Starting and advancing the traversal

For a nonempty parsed body, acquire the source list under LIST-13, validate its
owner/field/incarnation identity, and establish a cursor with boundary `q = 0`.
Save the checked collection identity, `q`, and current-membership/origin state in
compiler-visible control storage. Release the list before entering user code.
No entry array, target values, or visit history is copied. Setup and cursor
storage are `O(1)` per active loop, independent of list length.

Before each body iteration, acquire the list and validate its incarnation. Let
`L` be its current ordered membership array and `n` its current length. If
`q == n`, close this cursor and finish. Otherwise select exactly `L[q]` and
increment `q` before checking the selected target or entering the body. Skip a
reference entry with an empty/stale/deleted referent and continue from the new
boundary; a skipped entry never activates or changes the binding. For an eligible
membership, copy its target handle into the binding, record its membership
origin, release the list, then enter the body. Activate a new local cell only for
the first eligible entry. A live but busy target is not silently filtered: an
actual invocation/read through the binding uses ordinary refusal. If no candidate
is eligible, finish without activating a new local or modifying a reused
variable. No target is cloned, executed, or kept acquired by selection. Each
candidate selection and boundary advance costs `O(1)`, excluding body/lifecycle
work; the traversal itself uses no native index/truth conversion.

### The boundary and list mutations

The invariant is `0 <= q <= n`: the future portion of this traversal is exactly
`L[q..n)`. Entries before `q` are excluded from its immediate future, not
necessarily already visited. LIST-13 adjusts every active or suspended cursor
for this exact collection as part of each committed structural mutation:

| Committed operation at zero-based position `i` | Boundary adjustment |
|---|---|
| Insert, including append | If `i < q`, increment `q`; otherwise leave it unchanged |
| Remove | If `i < q`, decrement `q`; otherwise leave it unchanged |
| Clear | Set `q = 0` |
| Swap, capacity change, owned payload update, or atomic reference-slot replacement | Leave `q` unchanged |

An insertion exactly at the boundary belongs to the future portion. Immediately
after selection, before any intervening mutation, a new entry before the selected
entry is excluded, while one immediately after it or at the end is included.
The boundary is a position, not an anchor attached to that entry: after mutations
the numeric rules alone decide eligibility. Removing an entry before the boundary
does not skip the surviving entry at the boundary. Removing a future entry
prevents its selection.
After removal of the current entry, insertion at the resulting boundary is
visited. Clear followed by append within the still-running body makes the new
entry eligible; a closed cursor is never reopened by later insertion.

Swap preserves the numerical boundary and changes the future portion according
to the new order. A swap across the boundary can repeat a previously visited
membership and exclude a not-yet-visited one. This is permitted positional-scan
behavior; there is no hidden visited set. With only insertion/removal and no
reordering across the boundary, each membership is considered at most once.
Different reference-list entries remain distinct even when their handles match.
Atomic reference-slot replacement creates a new membership at the same position
without moving the boundary: replacement in the past stays excluded, replacement
in the future remains eligible. Explicit remove followed by insert is a different
sequence of structural operations and follows both adjustment rules. Updating an
owned element under its stable identity preserves its membership.

### Nested traversals

Nested foreach statements establish independent boundaries at zero over the
then-current list. A mutation adjusts all of them that name that same checked
collection, including cursors suspended by `%`; each may classify the insertion
as past or future differently. Whole-list replacement/reset changes its
incarnation and invalidates existing cursors rather than adjusting or restarting
them. Ordinary insertion, deletion, replacement of one reference slot, and
reordering preserve the collection incarnation.

### Termination and candidate bounds

For a termination argument, let `R = n - q`. A candidate selection decreases `R`
by one; insertion into the future portion increases it by one; removal there
decreases it; edits before the boundary and swaps preserve it. Therefore, if each
body finishes and only finitely many future insertions occur, the number of
candidate selections is at most the initial length plus their count. Skipped
empty/stale entries count as selections. The language imposes no unconditional
finite-pass guarantee: continually appending work can keep a traversal running.
The same potential bounds candidate counts with swaps, but does not imply
at-most-once membership visitation when entries cross the boundary.

### Access after an entry disappears

An iterator-installed binding has a compiler-visible origin association:
collection identity, membership identity, and installed target handle. Before
entry, resume, or retained read through that still-associated binding, validate
the membership and its referent before evaluating the target's inputs, resetting
its interface, executing it, or transferring its results. Missing membership or
target follows that operation's ordinary `:` refusal. This also protects a
reference-list binding after its entry is removed while the external object
remains alive. List operations and other independent statements may still run
after removal of the current entry.

An explicit assignment/output replacing the binding with a different handle
detaches that association and deliberately makes it an ordinary independent
reference; a same-handle no-op retains it. Copying the handle into another
variable likewise creates an ordinary reference, without a membership origin.
Such independent references to a still-live external object are not revoked by
removing its reference-list entry. The absence guarantee applies to checked list
selection and associated iteration bindings, not global destruction of an
otherwise-live object. Nested reuse installs its new origin in the same active
binding cell without restoring an old handle or origin on return.

### Suspension and resumption

On resumption from `%`, validate active collection identities outermost first.
If a current membership or referent no longer exists, abandon the remainder of
that iteration and advance that loop from its already-adjusted boundary;
discard any nested body contexts being left before executing user code.
Otherwise continue at the saved body position. A replaced/destroyed collection
or failed internal list acquisition is a technical failure at the foreach
header. Ordinary mutations preserve the cursor. No raw buffer
pointer or acquired list/element survives a body or yield. Current-instance field
access is authorized access, not external reentry of its running owner.

### Exits, cleanup, and reused bindings

`^` exits the innermost loop; early `>` exits the invocation's loops. Completion,
break, return, fresh `!`, failure, and disposal discard the exited cursors and
local cells, without destroying referenced targets. Drop a binding's origin
association only when its last active foreach role ends; an inner reused role's
exit does not erase the origin held by a still-active outer role. Reused
variables keep their last stored ordinary handles; a traversal
with no eligible entries leaves them unchanged. Rebinding an iterator variable
does not change its boundary. `%` preserves cursors, active cells, and origin
associations under CONT-02/CONT-04, but preserves no busy flag or access grant.

### Empty bodies

For an empty parsed foreach body under SYN-01, resolve/validate the collection
once and continue after `}` without establishing a cursor, visiting entries,
activating a local cell, or changing a reused variable, even if the list is
nonempty. Preserve failures
of that collection access and emit the ordinary empty-body warning. A body
containing an ordinary declaration is nonempty even if it emits no runtime work.

## Access protection and safe failure

<a id="list-10"></a>

**LIST-10.** Service opening resolves the list location once, validates its owner
identity, protects that owner against replacement/destruction, and acquires the
list as busy. Ordinary access to a field of the currently executing owner uses
the exact self-owner authorization under REF-08; this does not bypass an already
busy list. Another acquisition of that list refuses before argument/default
preparation. Only a method selected directly from this already acquired service
parent inherits that parent's exact acquisition token. An arbitrary call from a
method result's body, another alias, or the same source module cannot reenter by
claiming the caller's identity.

Generated selection helpers resolving that method's direct input sources,
indices, receivers, and preflight checks may borrow its exact token for that
same collection. Thus `< &items[position]` can use an already acquired source
list without reacquiring it. This structural-access grant is never passed to
executed Ama/native result bodies, user callbacks, or unrelated operations;
they must acquire their own target normally and cannot reenter the busy list.

Keep the list busy and owner protected through all selected methods, their
effects, result transfers, and nested result-action subtrees, until the entire
service parent finishes. Methods act sequentially under that same token; they
do not temporarily publish an available list between sibling actions. Cleanup
releases only the initiating operation's protections before running a refusal
handler. No busy state or runtime token is copied as application data.

After preparing its declared inputs, each method preflights positions, source
availability over COPY-07's whole copy boundary, and all required destruction
subtrees. An invalid position, unavailable nonempty owning-copy source, or
busy/incompatibly acquired deletion subtree is an expected refusal before
method entry, mutation, or output actions. Prepared inputs are disposed, result
receivers remain unchanged, and the method's own `:` runs under CALL-09 after
the initiating service tree is released. Pending sibling/ancestor actions are
skipped; previous committed actions remain committed. A parent's handler does
not catch a descendant method's refusal. Without its own handler, ordinary
unhandled-acquisition failure applies. Opening an unavailable list is instead
the parent operation's acquisition refusal and uses that parent's own handler.

Prepare allocations, instance construction/clone state, and all required
identity counter values before publishing an insertion. Expected refusal leaves
order, membership, ownership, element state, and every cursor unchanged.
Preparation may run its required input/lifecycle code under the normal access
rules. Publish committed structural changes and LIST-13's cursor corrections
under the same list acquisition, with no user-code interleaving between the
first committed structural change and the last correction. Complete publication
before output actions or further application execution. Allocation,
construction, copying, non-index technical conversion, and counter-exhaustion failures are technical
errors, never successful no-ops, false results, or `:` refusals. Once destruction
begins, destructor failure uses the terminal cleanup protocol rather than a
promised rollback of already destroyed instances.

### Removing owned values and reference entries

<a id="list-06"></a>

**LIST-06.** Removing an owning-list entry destroys its instance and invalidates
references to it through the generation protocol. Removing a reference-list
entry retires only its membership; it neither destroys nor acquires its target.
An owned element with a busy or incompatibly acquired instance anywhere in its
destruction subtree cannot be removed. Preflight that entire subtree under
REF-08 before retiring the membership or changing element state. `clear`
preflights every affected owned subtree before destroying any element.

Retained reads and indexed operations protect their selected instances under
the normal acquisition rules. List-buffer addresses are temporary borrows and
must not survive reallocation or shifts; they are not instance references or
iterator identities. Foreach bindings copy checked handles, so deleting a
currently bound owned element makes that handle stale; subsequent target access
must refuse before executing that target. A removed reference-list membership
does not revoke independent references to its surviving target.

## Length and whole-list copies

<a id="list-11"></a>

**LIST-11.** Length is stored structural metadata, updated at each successful
insertion/removal and initially equal to the number of initialized entries.
`length !` creates an ordinary result from that metadata. There are no retained
whole-list results to read with `>>`. Whole-list value copying copies ordered
target values according to ownership/reference-copy rules, establishes the new
length, and creates independent destination membership records. It never
inherits busy state, acquisition tokens, or active iterators merely because
they are associated with the source list. Explicit continuation copying handles
saved cursors separately under CONT-04.

## Membership identity and traversal bookkeeping

<a id="list-13"></a>

**LIST-13.** Every membership has one exact identity for its lifetime.
Append/insert creates a fresh membership; remove/clear retires the affected
memberships; swap and position shifts preserve them. Replacing an owning
element's payload under its stable instance identity preserves its containing
membership. Assigning a different handle to a reference-list entry retires its
old membership and publishes a fresh membership at that position; assigning
the identical full handle is a no-op. Atomic reference-slot replacement changes
neither length nor order positions and leaves every boundary unchanged; it is
not defined as separate remove/insert operations. Capacity changes never alter
membership identities.

### Short acquisitions for traversal

At foreach entry, acquire the list long enough to validate its checked location
and establish the live boundary from FLOW-05, then release it before application
code. Each selection briefly reacquires the list, validates its incarnation,
selects its current order entry at `q`, and advances `q` before entering a body
or skipping an empty/stale reference. If `q` equals current length, close the
cursor. Duplicate reference entries remain separate candidates. A live but busy
target is not filtered: an actual attempt to execute/access it uses normal
acquisition and refusal rules.

### Fixed cursor slots and two exact correction strategies

For each list field, the compiler enumerates the fixed set of lexical foreach
sites whose resolved source is that field. Each owning instance has a generated
cursor slot for each such site under RT-02, including an active/suspended marker,
checked collection identity, boundary, and current-membership state. Let `K` be
the number of these lexical slots, including currently inactive ones, and `A`
the number currently active or suspended for this exact collection. Choose and
document either of the following exact correction strategies:

- **Fixed scan:** for each committed insert/remove/clear, inspect the `K` slots
  in lexical order. Apply FLOW-05's exact correction only to active or suspended
  cursors whose full collection identity, including incarnation, matches the
  mutated list. This costs O(K) and needs no separate registration structure.
- **Active registry:** maintain an intrusive per-list chain of exactly its `A`
  active/suspended cursor slots. A cursor uses compiler-visible previous/next
  links and the list uses a head selector. Links identify explicit cursor slots
  by checked owner identity and compiler-known lexical selector, never opaque
  native pointers. Enroll on activation and unlink on closure or exit in O(1).
  A committed insert/remove/clear visits each registered cursor once, validates
  its exact collection association, and applies the same correction in O(A).
  Registration order is explicit and deterministic; it cannot change the
  numerical correction or expose partially corrected state to application code.

The registry invariant is a bijection between live registrations and cursor
slots active or suspended for that exact full collection identity. No cursor
appears twice. Inactive, closed, other-instance, and obsolete-incarnation cursors
are never adjusted or reactivated by either strategy. Before unregistering or
following a link, validate its identity; after a containing instance is destroyed
or reset, no registry may retain a link to its retired cursor storage. Scope
exit, fresh `!`, break, return, completion, failure, disposal, and whole-list
replacement must remove all affected registrations before invalidating storage.
Yield keeps the registration because the saved boundary still needs correction.
Whole-list replacement does not silently reopen old cursors: invalidate them
under the ordinary incarnation rules after detaching their registrations.

Registrations are runtime bookkeeping, not copied application data. Ordinary
copying creates no registrations. A `%` continuation copy first performs
CONT-04's exact collection/cursor/origin remapping and then establishes one fresh
registration for each copied active/suspended cursor against its mapped
collection. It copies no previous/next link or acquisition token. Failure
cleanup removes any partial registrations. Publish registrations with the
destination's completed state, before its saved continuation can execute and
without interleaving user code; do not expose constructing cursor storage in a
live collection's registry. Resume validates the saved cursor
and existing registration without registering it a second time; a removed
current membership still follows FLOW-05's skip-and-advance rule.

Append requires no enumeration because its position is always at or after every
valid boundary. Swap, capacity changes, owned payload updates, and atomic
reference-slot replacement likewise need no numeric correction. Fresh-loop
setup and each selection cost O(1), with O(1) storage per lexical cursor
independent of list length. Neither strategy uses a target search, a global
cursor registry, or a saved array of list entries.

**Explanation.** A module may have 100 lexical traversals of a field while only
one traversal is currently active: the registry corrects one boundary instead
of inspecting 100 slots. With only one or two lexical sites, a fixed scan can
save link storage and registration work. Both are valid representations of the
same boundary semantics. Select by an explicit implementation rule or proved
layout capability, and measure the tradeoff; no runtime guess may change which
entries are visited. Tests must exercise nested and suspended cursors, empty
bodies, early exits, replacement, continuation copies, and failure cleanup under
the implemented strategy, including reduced-width identity exhaustion. A
component offering both strategies must demonstrate equivalent semantic
outcomes for these cases; implementing both is not a conformance requirement.

### Publishing one complete mutation

Apply all order, length, membership-position, and cursor changes as one protected
publication, with no user code observing or interleaving a partially corrected
state; complete it before releasing the list or executing output actions.
Required preparation/lifecycle work follows LIST-10 and the normal lifecycle
contracts. A refused or failed preparation changes no cursor. A clear may reset all matching
boundaries directly to zero as the exact aggregate of its removals. If terminal
destructor failure prevents a successful commit, use the ordinary terminal
cleanup protocol; do not promise continued iteration over partially destroyed
storage. A whole-list replacement changes incarnation, leaving old cursors
invalid rather than correcting them to the replacement.

Insertions before a boundary remain behind it; insertions at or after it are
future work, including append. Deletions before it shift it back; deleted future
entries are never selected. Swap keeps the numerical boundary and may repeat or
exclude memberships crossing it. No visited set, fixed entry count, or implicit
termination limit is used. Ordinary structural mutation neither invalidates nor
restarts the cursor. No exclusive list acquisition is retained across a body
statement or a `%` suspension. FLOW-05 defines local activation, early exits,
potential-based candidate bounds, and saved-body resumption. Before resuming a
body, revalidate its current membership and target; a deleted/retargeted
membership or empty/stale target skips the remaining body and continues from its
already-adjusted boundary, even if no candidate remains there.
An unchanged iterator binding also carries FLOW-05's exact membership-origin
guard, so accessing it after removal refuses even when an independently owned
reference-list target remains alive. Independent aliases are not globally
revoked. CONT-04 defines continuation-copy cursor remapping and numeric-boundary
preservation. Independently copied lists receive independent membership identities.

### Collection replacement and counter exhaustion

Whole-list replacement, interface reinitialization, or replacement of a
containing instance root creates a fresh collection incarnation at every
affected live field, even for equal/empty contents. An active cursor with an old
incarnation must fail on validation; it must not silently restart against the
replacement. Exact whole-list self-assignment does not replace the incarnation.
Owner destruction/reuse is distinguished by its ordinary generation. Reserve
every required incarnation/record generation before a commit. These counters
never wrap, reset for a live identity, or revive retired identifiers; exhaustion
is a technical failure preserving the old collection. Counter widths are an
implementation detail; reduced-width exhaustion tests must cover insert/remove,
reference-entry replacement, whole-list reset, and containing-root replacement.
No length comparison, target-handle search, timing rule, probabilistic hash, or
blanket rejection of structural mutation may substitute for these identity checks.

## Required representation and complexity

<a id="list-03"></a>

**LIST-03.** A list's logical order is a contiguous ordered array of membership
identifiers. Its physical order array may store only local record-slot indices.
A per-list record pool maps each live local index to a checked target handle,
its current array position, and a nonzero record generation. The complete
membership identity comprises checked owner type/slot/generation, fixed
list-field selector, collection incarnation, local record index, and record
generation. Equal local counter values in different lists never denote the same
membership. A record-slot generation never wraps or repeats within its
collection incarnation; retiring a record invalidates its old identifiers
before that slot is reused. Exact membership lookup is O(1), without searching
for a target handle. Duplicate handles in a reference list have distinct
membership identifiers.

The compact order entry obtains its owner/field/incarnation from the acquired
list and its generation from the selected live record. This reconstruction is
valid only for a current order entry under that list acquisition. Every saved
cursor current-membership value, iterator-origin association, or other identity
kept beyond that acquisition must retain the full checked identity, explicitly
or by an equally exact non-ambiguous representation. It may not recover a saved
generation from whichever record later occupies that index. In particular,
record-slot reuse and duplicate target handles cannot make a removed membership
look unchanged.

For every current order position `i`, its local index selects exactly one live
record whose stored position is `i`; that record appears at no other order
position. Its reconstructed full identity is the membership at `i`. Preserve
these invariants at every publication. Narrow local index widths are permitted
only with checked representability and exact conversion. Before publishing an
index beyond the current width, perform checked growth/re-encoding; inability
to allocate that representation follows the ordinary technical resource-failure
contract. A narrow encoding must not silently impose a smaller language-level
length limit or cause truncation, wrap, or aliasing.

**Explanation and implementation.** Moving an order suffix during insertion
need only move these local indices. Owner identity, field selector, and
incarnation already belong to the list header; repeating them in every array
entry would spend memory and copy bandwidth without adding information. Keep
generations in the record table, perform the index shift, update the affected
records' positions, and publish the whole mutation under LIST-10/LIST-13.
For example, removing one of two reference entries pointing to the same target
retires just its full membership, even if its local index is immediately reused.
Tests must retain that retired identity and reject it after reuse while the
other entry remains usable. No particular machine-word size or byte saving is
part of the language contract.

In the ordinary representation, instance states stay in their module's
stable-address pool, outside both the order array and membership records.
RT-02/GEN-03 permit a different physical representation only when their complete
proof obligations hold; iterator-reference observations remain part of that
proof. Growing or reordering list storage must not move an observable live
instance payload. Keep each live record's position equal to its entry's
current array index after every committed mutation. Lists must not use linked
occupied-slot chains to represent ordinary positional order.

Use geometric capacity growth for amortized constant-time append; the exact
factor is an implementation choice. Check allocation and size arithmetic.
Allocate capacity and record/pool slots in batches rather than allocating a
separate heap node for each insertion. Native resources may require their own
allocations.

### Costs and stable identity

<a id="list-05"></a>

**LIST-05.** An index denotes current order, not stable element or membership
identity. Indexing and membership lookup are O(1); append is amortized O(1),
and swapping two positions is O(1). Insertion or order-preserving removal can
move O(n) order entries and must update the affected records' positions. Add
O(K) for the fixed cursor scan or O(A) for the exact active registry on
insert/remove/clear under LIST-13. Here K is the number of lexical cursor slots
associated with this field in its owner type and A is the active/suspended count
for this exact collection. Registry activation and removal cost O(1) per cursor.
Append, swap, and atomic reference-slot replacement need no numeric boundary
correction. These bounds exclude element lifecycle costs; foreach setup and
per-cursor storage are O(1), without a list-length-dependent allocation.

Removal preserves the order of remaining entries. References to surviving
instances follow their targets rather than their former positions. Membership
identifiers follow their specific entries, including duplicate reference-list
entries; a target handle alone cannot identify which duplicate was selected.

### A queue is a separate collection workload

<a id="list-07"></a>

**LIST-07.** Frequent removal from the beginning is a queue workload. Libraries
may provide a circular-buffer queue over the same instance handles and pools,
without changing ordinary list order or adding language syntax. A linked
ordering structure is not required to optimize this separate workload.

## Explanation: templates and the list implementation

**Informative.** The boundary between module templates and the list primitive is
defined by [TPL-09](templates.md#tpl-09). Templates make it possible to describe
ordinary collection wrappers and algorithms once for different element types.
For example, a wrapper can keep a typed list as its state, and a filtering
algorithm can use the existing traversal and insertion operations. These uses
build on the list primitive; they do not require implementing that primitive
again for each element type.

Parameterizing the primitive itself is a different implementation question.
Its element type determines construction, copying, destruction and checked
reference types. Its storage also needs the membership records, stable target
identities and order array described by [LIST-03](#list-03). A template can
organize code around those responsibilities, but naming an element parameter
does not supply typed storage or lifecycle operations. Defining a primitive
list solely in terms of another instance of that same primitive list would
leave the storage foundation unexplained. A realization instead needs an
explicit compiler/runtime foundation for those operations. Persistent Ama
references remain compiler-visible typed state; placing them in opaque native
payloads is not a way to provide that foundation.

### An empty list still has an element type

**Example.** This declaration fragment assumes that `/example/demo/v1.0.0/src/shop/item` is a concrete
module type:

```text
items[]
/example/demo/v1.0.0/src/shop/item
```

The compiler knows the exact element type from the declaration even when the
list has no entries. A parameterized list implementation therefore has type
information available before any insertion; it need not obtain a sample element
or wait for the first method call. Named application and composition follow
[TPL-07/TPL-08](templates.md#tpl-07), with static graphs under
[TPL-12](static-results.md#tpl-12) and complete ordinary declaration expressions
under [TPL-15](templates.md#tpl-15).
Choosing the list implementation interface,
the exact formal receiving that declaration-supplied identity and the primitive
storage/lifecycle operations remains
[AMB-006](../../AMBIGUITIES.md#amb-006). This explanation does not select that
interface, introduce a hidden sample value or claim that the bridge has been
implemented.

### Factoring code preserves the list contract

The six entries in [LIST-04](#list-04) remain intrinsic typed service entries.
An ordinary template wrapper can expose its own module interface, but that does
not turn the primitive's methods into owned module instances, retained results
or resumable objects. Similarly, factoring their implementation does not change
the service-frame lifetime of method results or make those results available
through a hidden method object.

The relevant implementation obligations remain those already defined in this
chapter:

- [LIST-10](#list-10) governs busy acquisition, source and destruction preflight,
  result publication, refusal handling and cleanup. A wrapper does not grant
  its callbacks access to the primitive's private acquisition token.
- [LIST-13](#list-13) governs membership identity, live traversal boundaries,
  correction during structural mutation and suspended cursors. Ordinary list
  copies and explicit `%` continuation copies retain their different contracts:
  a continuation copy needs the specified cursor, collection and origin
  remapping, rather than copied acquisition tokens or native pointers.
- [LIST-03](#list-03), [LIST-05](#list-05) and [LIST-13](#list-13) fix the
  representation invariants and complexity bounds. Template-based factoring
  does not replace direct membership lookup with target searches, introduce
  per-entry linked ordering or remove the required cursor-correction costs.

These distinctions allow the implementation to share suitable code while the
source-level list behavior stays defined in one place. They describe a design
boundary, not evidence that a complete list implementation can already be
rewritten as ordinary Ama templates.
