# Handle-vector storage foundation

The [list specification](../../spec/language/lists.md) requires typed intrinsic
methods, checked memberships and live traversal. Those services remain T075–T084.
The retained C component below implements storage/lifecycle operations only; it
is not an executable list interface or evidence of foreach support.

T136 removed the runtime action/request dispatcher, fixed positional semantic
bindings, configurable action/index/condition/outcome type mappings, list action
IR and the action/outcome library modules. `AmaHandleList` retains its vector,
length, capacity and acquisition bookkeeping, without retained refusal/outcome
fields. Copies transfer data, not an old completion flag. Whole-list execution
and retained reads now fail semantic checking before unsupported IR is created.

## Domain and authority

`src/runtime/lists/list.h` exposes two direct C operations: insert before one
position, and remove one contiguous range. `AmaListStorage` borrows a live
owner's exact field, matching element type and ownership. Its caller supplies
exclusive authority and a handle-vector representation without cursor or
membership records. It is not a language-level method frame or an action value.
The storage object owns no persistent source/result and changes no Ama body
execution position. The world prevents lifecycle reentry while an edit runs.

The foundation's limitations are explicit: collection incarnation, membership
retirement, cursor correction, typed method results, checked list acquisition
and the integer length bridge are still required before language list services
can call a completed representation. Do not treat this vector as the final
membership order array from LIST-03.

## Capacity and stable payloads

`src/runtime/lists/capacity.c` checks handle-count and byte-size bounds before
allocation. Growth copies the existing handle prefix before replacing its old
buffer; allocation failure preserves contents, count and capacity. Only handles
move. Live instance payloads remain in stable pool slots.

The invariant is `count <= capacity <= SIZE_MAX / sizeof(AmaHandle)`. Starting
at four and doubling, with a final checked saturation, gives a geometric bound
on copied handles and amortized O(1) growth per append, excluding lifecycle
work. Initial construction and whole-list copying establish the invariant;
disposal clears the vector. Capacity reservation can succeed before a later
constructor fails; that failure preserves logical contents without promising
unchanged reserved capacity.

## Preflight, construction and cleanup

Insertion requires `position <= count`. Range removal requires
`position <= count` and `range_count <= count - position`; subtraction follows
the first check, avoiding overflow even for `SIZE_MAX` inputs. Violations return
`AMA_INDEX_BOUNDS` directly. Availability refusals also return their status,
without modifying list completion state or turning refusal into success.

Owned insertion preflights a nonempty source before constructing an independent
ordinary clone. Empty source constructs initial state. The existing object
builder remaps internal references and clears saved positions. Publish the
complete constructed tree and its owner ledger before publishing its handle.
Reference insertion copies the complete supplied handle, including empty/stale
handles, without acquiring the target or running constructors.

Owned removal preflights every affected ownership tree before its first
destructor. Preparation refusal therefore has no destructive prefix. Removal
continues terminal cleanup after destructor failure, preserves the primary
failure and adjusts the parent ledger by the removed owned roots. Reference
removal neither acquires nor destroys referenced targets. All exits after a
successful operation setup release preflight protection and scratch storage.

Insertion shifts exactly the suffix `[position, count)` right by one. Range
removal shifts `[position + range_count, count)` left by `range_count`.
Consequently surviving handles retain their relative order and payload identity;
removed owned roots become stale, while unrelated references remain unchanged.
Position checks and bookkeeping cost O(1); suffix shifts cost O(n), excluding
owned-tree preflight/construction/destruction. These are storage laws, not the
additional membership/cursor obligations of the complete list contract.

## Executable evidence

Run `make -C tests/unit/runtime/lists` independently. Its seven named cases cover
geometric growth and allocation/size limits; ordered insertion/removal and stale
owned roots; busy descendants before any destruction; empty/busy reference
storage; every allocation-failure prefix of default insertion; terminal cleanup;
ordinary cloned state with remapped self/child references and cleared resume;
and reentrant/failed-world rejection without allocation or mutation.
`make -C tests/unit/runtime/objects` and the generated-project suites compose the
same retained lifetime/copy/transfer components through their real parents.
The complete `make -j4 test` is the acceptance check after removing old API
consumers and updating the runtime C assembly's source membership.

CLI tests in `tests/unit/driver/run` reject all four retired configuration
options before opening roots. `tests/unit/semantic/types` rejects whole-list
execution and retained-result bindings before planning. Scalar native-library
compositions continue to compile and run; the old enum-literal expectation was
removed together with its obsolete module.
