# Ama: modules, states, objects, and lists

This document records the design discussion and the work it suggests. It is a
design roadmap, not the current language specification. `README.md` and
`EBNF.md` describe implemented behavior. Nothing in this document changes the
compiler by itself.

## The concept, briefly

A module describes both executable behavior and the structure of its state.
The module is a type. An object is one instance of that state. Many objects can
use the same module code without sharing their own variables.

Calling an object means executing the module over the selected state. The
state stays with its owner. The implementation brings code to the data rather
than copying a large tree of data into a module's global variables.

The caller supplies parameters, the body executes, and requested results are
copied back. Internal state survives calls. Reading a stored result does not
execute the module.

Declarations describe state; they are not repeated initialization commands.
Creating an instance initializes its variables and owned children once. Calling
it does not recreate them. A separate constructor section is unnecessary.

Cloning is explicit. It copies current values and owned child states, including
owned lists. References to independent objects continue to identify those same
objects. Cloning does not execute the module body.

A list owns a runtime-sized sequence of states of one module type. Its storage
can grow automatically. Calling an element executes its module over that
element's state, without copying the entire list or element tree.

Reactivity belongs to application code. A module can compare incoming values
with saved values and decide which children to call. Every invocation executes
the body, even when its arguments equal those of the preceding invocation.

Composition means owning a child state. Aggregation means referring to an
independently existing object. References identify whole typed objects and expose
their public interfaces, not memory addresses or individual internal variables.
An owned declaration uses `.sys.obj`; a reference declaration uses `&.sys.obj`.
Reference variable names must also start with `&`, as in `&target`; this is
checked syntax, not an optional naming convention. The expression `&my_obj`
obtains a reference to an owned `my_obj`, whereas `&target` reads an already
declared reference variable. Neither operation copies state or transfers
ownership.

The runtime checks each call and marks the selected instance busy until the call
ends. An attached `:` branch handles failure to start a call, such as a missing
or busy object. The module body needs no manual availability check. Removing an
object invalidates existing references; they never silently select a replacement.

An explicit `@enum` selector will distinguish modules that declare named
constants from executable modules and unfinished drafts. Replacing `@Ama` with
`@do` was suggested, but the replacement has not been selected.

## Decision status

### Agreed direction

- A module can serve as the type of independently stored states.
- Execution works over a selected state; calling does not require copying it.
- Application code controls reactions and the order of child calls.
- Repeated identical calls still execute the body repeatedly.
- Declarations establish persistent state; ordinary calls do not recreate it.
- No separate constructor annotation or section is needed.
- Explicit state cloning is a required capability.
- Lists must support an element count unknown at compile time.
- Lists should be usable as owned parts of another module's state.
- List operations can be requested through parameters of one list module.
  Separate procedure modules for every operation are not required.
- Typed references use `&.module.path` declarations and `&instance` acquisition.
- Every reference variable or parameter name starts with `&` in declarations
  and uses, including named argument labels and calls through references.
- Reading an existing `&name` copies its reference value; it does not construct
  a reference to a reference.
- A reference neither transfers ownership nor extends the target lifetime.
- The runtime controls per-instance busy state; module bodies do not maintain it.
- Calls validate access themselves; preliminary availability checks are optional.
- A `:` alternative attached to a call is the chosen direction for handling
  inability to start execution.
- `@enum` with an empty body is the chosen direction for named constants.
- Preserve the line-oriented style: declarations and argument values are
  separate lines; arguments remain nested beneath their call.

### Proposed rules requiring confirmation

- Clone owned children recursively and preserve external object references.
- Preserve the current per-call defaults for parameters and results while
  introducing persistent internal variables.
- Give a system list an action parameter whose values come from an enum module.
- Use checked handles for independently addressable objects.
- Forbid structural changes to a list while an element is executing.
- Let a list select an element through a cursor-like result.
- Keep an instance per module as the default target for existing alias calls.

These are design candidates, not all accepted language rules. In particular,
the cursor design, exact failure-status syntax, and destruction API still need
practical examples. Checked handles and explicit destruction are the current
working design; their representation and full resource policy remain to be specified.

## Detailed model

### 1. Separate code, state, and selection

Three entities must stay distinct:

| Entity | Purpose |
|---|---|
| Module/type | Describes state layout, interface, and executable behavior |
| State instance | Holds values belonging to one object |
| Instance selection | Identifies the state over which a call will execute |

A module alias currently identifies code. An owned child declaration must also
establish which state belongs to its parent. An external reference selects an
independent state. In the proposed syntax, `.module.path` declares an owned
instance, whereas `&.module.path` declares a reference. This changes the meaning
of today's alias declarations and requires an explicit migration decision.

The implementation can generate a structure for each module and pass its
address explicitly to its executable function:

```c
typedef struct {
    size_t increment;
    size_t value;
    size_t accumulated;
} CounterState;

void counter_execute(CounterState* state);
```

This is an implementation sketch, not a proposed native ABI. The pointer is
an implementation detail, not an Ama value. Public parameters and results and
their reset behavior may eventually use a separate call frame.

Avoid a mutable global "current instance" and avoid loading selected state into
the existing global variables before a call. Explicit contexts make nested
calls and instance independence easier to understand.

### 2. Module identity

A normalized path relative to the compilation namespace identifies a module.
The standard library already shares that namespace with project modules;
duplicate paths are rejected.

A compiler-assigned integer can identify the type inside one compiled program.
The full path remains available for readable diagnostics. If identifiers are
assigned during scanning, they need not remain stable across builds.

A path hash is an alternative lookup mechanism, not proof of uniqueness:
collisions must be checked against full names. Sorting paths before assigning
integers is another option if reproducible numbering becomes useful.

Neither a path nor a numeric ID establishes serialization compatibility. A
future saved-state format would also need a layout/schema version. Persistent
type IDs, persistence, and separate compilation are not prerequisites for the
first implementation.

### 3. Creation, calls, and cloning

| Operation | Intended effect |
|---|---|
| Create | Initialize a fresh state and its owned children |
| Call | Supply inputs and execute against an existing state |
| Read result | Read the last stored public output without execution |
| Clone | Create a distinct copy of current owned state |
| Destroy | Release owned resources under the chosen lifetime policy |

Creation and cloning are different. A new counter starts from declared initial
values; a clone of a counter at 57 starts at 57.

There is no constructor section. Numeric declarations establish initial values,
owned child declarations establish child states, and list declarations establish
empty lists unless an initial length is explicitly specified. Complex application
setup can be requested through an ordinary call; no hidden one-time body is
introduced.

The current language resets parameters and results to defaults and locals to
zero on calls. Persistent object state therefore requires a real semantic
change, not just a different allocation method. Decide how persistent internal
variables are declared and how they differ from temporary values inferred in
the body. Do not silently turn every existing temporary into persistent memory.

The candidate call protocol is:

1. Select and validate the target instance.
2. Establish per-call parameter/result defaults.
3. Copy supplied parameter values.
4. Execute the body using the selected internal state.
5. Copy requested results back and retain the resulting object state.

If defaults remain per-call, omitted inputs mean their defaults, not "retain
the previous input." An application that needs a previous input saves it in
internal state. Partial-update behavior must be explicit in its interface.

The exact initialization declarations, temporary-variable rules, and public
result access syntax remain open.

### 4. Reactivity and repeated requests

There is no automatic dependency propagation, change detector, or scheduler.
The module body decides whether to compare values, update children, or do
nothing. Comparison with prior values is optional application behavior.

Two calls with the same append request execute the list body twice. The list
can append twice or implement an explicitly documented deduplication policy.
An action is a request on each invocation, not an edge detected by comparing
the current parameter against its previous value.

Reading a result should not execute code, allocate objects, or update children.
Whether a requested operation reads outputs or invokes behavior must remain
visible in source.

### 5. Composition, aggregation, and clones

Ownership forms a tree even when references form a graph:

```text
car owns engine
car owns wheels
car refers to fuel_station
truck refers to the same fuel_station
```

Under the proposed clone rule, cloning the car copies its engine and wheels,
but both cars refer to the same station. Destroying a car releases its owned
children and does not destroy the station.

An object cannot be owned by two parents. Shared relationships are references.
A type cannot directly contain itself by value through an infinite chain of
owned fixed-size children. Dynamic lists and external references require
separate rules if recursive state types are later allowed.

Do not define cloning as unrestricted byte copying. Owned list allocations need
independent copies, while external references retain their targets. A partially
failed clone must release resources acquired during that attempt and leave the
source unchanged. Whether call-local inputs and retained outputs are included
in the clone must be specified along with the final state layout.

Deep copying an arbitrary external reference graph is outside the initial
scope. It would need cycle detection, preservation of shared identity, and
policies for external resources.

### 6. References and access duration

Prefer access to a whole typed object through its interface. Do not expose:

- Pointer arithmetic or conversion between numbers and references.
- References to individual internal variables.
- Arbitrary reinterpretation of one module's state as another module's state.
- Saved implementation pointers obtained during a temporary call.

#### Owned declarations and typed reference declarations

The agreed declaration contrast is:

```text
my_obj
.sys.obj
```

This declares an owned instance of the module at `sys/obj`. Here `sys/obj` is
an illustrative type path, not an existing standard library object API.

A reference parameter in `consumer` is declared as:

```text
@<
&target
&.sys.obj
```

Obtain and pass a reference explicitly:

```text
consumer
 &target < &my_obj
```

The consumer calls `&target` using the ordinary call shape. Copying a reference
preserves target identity; it does not clone state, transfer ownership, or keep
the target alive. The compiler checks the referenced module type. The runtime
checks that the instance still exists and is available.

Reference assignment must retain the prefix, for example:

```text
&reference &my_obj
&other &reference
```

The first assignment obtains a reference to owned `my_obj`; the second copies
an existing reference. These are future syntax sketches; declaration/inference
and empty-reference defaults still need specification. The earlier unprefixed
`reference &my_obj` sketch is superseded.

The compiler must require agreement between the `&` name prefix and the declared
reference type. A reference cannot be accessed by dropping its prefix. Resolve
`&name` against declarations so that acquiring a reference to an owned object
and reading a reference variable remain distinct. Do not introduce `&&name` or
implicit references to references. The combination of an owned `name` and a
reference variable `&name` in the same scope would make acquisition ambiguous;
choose an explicit rejection or name-resolution rule before implementation.
 References to owned children are no longer categorically
excluded: if exposed, they must be invalidated when the child or its owner is
destroyed. No raw pointer into an owner's allocation may escape.

#### Checked calls and the alternative branch

The proposed call syntax preserves separate argument lines:

```text
&target
 < 255
:
 handle_unavailable
```

The `:` branch extends the existing alternative/else idea. The working rule is
that it handles inability to start the call, including a missing or busy target.
The exact failure-reason binding is open; missing and busy must remain
distinguishable. A wrong statically known type is a compile-time error, not a
normal alternative. Runtime type validation is still required at native or
other dynamic boundaries.

Without an alternative, an invalid call should terminate with a clear diagnostic;
this default is a proposed policy. Failure to start does not assign call results.
An error after execution starts does not enter this branch. Busy cleanup does
not imply state rollback. Argument evaluation and failure ordering must also be
specified before implementation.

#### Runtime-managed availability

Availability belongs to each instance, not the module type. Conceptually a
slot has these states:

```text
Empty          no live instance
Ready(state)   available for execution
Busy(state)    executing; new access cannot start
```

The runtime validates the target and acquires it before resetting any call
parameters/results, marks it busy, executes the body, and releases it on every
return path. The address need not change: "taking state out of the slot" is a
model of exclusive access, not a required memory copy.

A call into another instance of the same module does not conflict with this
busy state. Reentry into the same instance is refused. Existing compile-time
module-cycle restrictions are not automatically lifted.

An `obj.@busy` property was considered as optional observation, not adopted as
required syntax. Users must never have to check it before every call. A busy
flag alone does not establish existence. The checked call combines validation
and acquisition; future concurrency would require making that operation atomic,
not relying on a separate check followed by an unprotected call.

#### Handles and deletion

A candidate runtime handle contains a type ID, slot index, and generation.
Access checks type, bounds, occupancy, and generation before resolving a pointer.
Deleting an object invalidates its handles even if its slot is reused. Generation
wraparound must not make an old handle valid again. An ordinary numeric value
must not be accepted as a typed reference merely because its bits match a handle.

The working deletion API is an explicit request to the owner, list, or pool.
These names are illustrative, not a settled interface:

```text
objects
 action < actions.remove
 id < object_id
 error > error
```

Before releasing anything, validate the target and check that it and all owned
descendants can be destroyed. Reject deletion if any are busy. Only then release
owned children and lists, invalidate their identities, and reclaim storage. A
failed preflight must leave the ownership tree intact. Native cleanup failure
policy still needs specification; successful preflight alone does not provide
transactional cleanup of arbitrary external resources.

External objects referenced by the deleted tree remain alive. Other references
to the deleted objects need not be found and cleared: subsequent calls through
them fail validation and take the call alternative. Reusing storage must issue
new identities. Initially prefer rejection of busy deletion to deferred deletion.

Distinguish two layers of failure: an unavailable pool takes the pool call's `:`
branch; a live pool unable to remove its requested target returns an operation
error such as missing or busy. The destruction authority of non-owning references
still needs to be defined; possessing a reference must not implicitly grant it.

Checked handles do not automatically reclaim leaks, prevent unintended shared
mutation, or provide thread safety. Explicit destruction is the current working
approach; region ownership or collection remain possible future policies. A
Rust-style user-facing borrowing system is not the intended model.

#### Alternatives considered and historical lessons

Several approaches were compared to keep the tradeoffs visible:

- Copying states avoids shared mutation but does not provide shared identity.
- Synchronized copies of one logical object require publication timing and
  conflict rules. They cost memory without removing shared-state semantics.
  Prefer one authoritative state for one shared identity.
- Immutable snapshots can be passed independently. Publishing a modified snapshot
  would require a separate version/conflict protocol, outside the initial scope.
- IDs resolved by an owner are a useful constrained form of aggregation. They
  avoid raw addresses but still need invalid-target behavior.
- Actor systems use opaque destinations, private state, and serial message
  processing. Their queues defer busy requests; our synchronous checked call
  instead refuses busy access. Queues and asynchronous answers are not required.
- An optional value models presence but not availability: absent and busy must
  be distinct. No monad abstraction is required to express the slot states.

The goal is one authoritative mutable state per identity, visible calls, and
runtime checks at access boundaries. This prevents unsafe memory access without
pretending that it eliminates application-level effects of shared mutation.

### 7. Dynamic lists of states

A list is an owned sequence of instances of one module type. Runtime length is
independent of the compile-time knowledge of the element layout.

The candidate system module is `sys/list`. Element type is fixed for a list's
lifetime. Initial length, if supported, creates that many independently
initialized states. Distinguish initial length from reserved storage capacity.

Potential inputs and results, with names still provisional:

| Member | Role |
|---|---|
| Element type | Establishes the list's state type; binding syntax is open |
| Action | Selects the operation for this call |
| Index | Selects a position for operations that need one |
| Source state | Supplies a state to clone when requested |
| Length result | Reports the current element count |
| Selected state result | Temporarily selects an element for calls/result reads |
| Error result | Reports invalid positions, allocation failure, or invalid use |

Useful actions include no operation, append initialized state, append clone,
select, remove, and clear. This is a candidate operation set, not a fixed API.
The module body interprets the action; operations do not have to be split into
different modules. Unused inputs and invalid combinations need documented rules.

A module that owns a list need not introduce a wrapper solely to obtain a typed
list. A wrapper is useful when it adds domain rules, such as a basket's pricing
or a warehouse's stock policy.

#### Storage and growth

A growable contiguous array is a simple first implementation. Capacity can grow
geometrically, but doubling is not a language guarantee. Allocation sizes must
be checked for overflow and failure. Growth moves states without executing,
cloning, or destroying their logical objects.

A plain stack is unsuitable for arbitrary removal because it implies reverse
creation order. Typed pools or segmented storage are alternatives when stable
addresses or arbitrary lifetime become important.

An active element must never be moved or destroyed under executing code. One
candidate rule forbids structural changes to that list for the duration of an
element call. Another implementation uses stable blocks; even then deletion
of an active element must be handled explicitly. Select a rule before exposing
list mutation semantics.

An index identifies a position, not persistent object identity. Removing an
earlier element can change what that index selects. Stable object references,
if required, need a separate identity mechanism.

#### Element selection without square brackets

Square-bracket indexing was suggested and rejected as an unnecessary syntax
addition. A cursor-like interface was then proposed: call the list with an index
and use its selected-state result to call the element.

The following is illustrative future syntax, not a valid current program:

```text
items
 index < position

items.item
 price < price
 quantity < quantity
```

Each parameter stays on its own line. A result access such as
`items.item.total` was suggested but has not been adopted.

A single mutable cursor creates a nested-traversal problem: an inner selection
can replace an outer selection. Decide whether selections are captured values,
scoped accesses, or mutable cursors, and define exactly when they expire. Do not
present cursor syntax as settled until nested examples work clearly.

Iteration syntax, bounds behavior, insertion order, removal behavior, and
mutation during traversal also remain open. These should be designed together
instead of inventing syntax in a large example.

### 8. Generators and deferred work

Generators produce a sequence incrementally and can avoid retaining every
produced state. They do not replace storage when independent objects must
remember their histories between visits.

General lazy evaluation is not required for objects or lists. It would introduce
questions about when stateful code and I/O execute and whether results are cached.
Deferred first-use initialization is a smaller optional optimization, but eager
initialization remains the simpler baseline. Neither is part of the initial
agreement.

### 9. Enum modules and implementation selectors

Use `@enum` with an empty body to explicitly mark a constant module. A proposed
example preserves the existing name/value declaration pairs:

```text
@>
none
0
append
1
remove
2
clear
3
@enum
```

Proposed validation rules: only result declarations with explicit numeric
values; no parameters, aliases, executable body, or mutation. Enum members are
available without executing the module, and the enum itself is not callable.

Initially, members can be named numeric constants rather than a new closed
value type. An action parameter receives the selected member's numeric value,
not the enum module itself. Unknown action values must produce a defined error.
Using zero for a no-operation default is convenient but remains an API choice.
Member access syntax, duplicate-value policy, and enum type checking remain open.

`@enum` must not change unfinished declaration files into valid modules by
accident: the selector is explicit. Empty bodies still require their selector.

The suggestion to replace `@Ama` with `@do` would emphasize execution rather
than implementation language. Other candidates were `@body`, `@proc`, and
`@code`. No replacement is accepted yet. `@C` remains the current native selector.

### 10. Native C and the standard library boundary

The compiler must know state layouts, element types, instance selection, and
generated initialization, cloning, execution, and cleanup operations. Ordinary
current numeric `@C` signatures cannot express all of these by themselves.

The standard library can implement allocation, growth, list operations, and
handle tables using compiler-provided descriptions or specialized generated
adapters. Static type specialization versus runtime type descriptors remains
an implementation choice. Do not assume that passing a module type is already
supported by the language.

Native module globals and C `static` variables are not automatically instance
state. Supporting stateful native objects requires an explicit context contract.
Files, sockets, and third-party resources require deliberate clone and cleanup
policies; byte copying is insufficient. Native code must not retain temporary
pointers into Ama states or lists. These constraints need contributor guidance
and an updated ABI before stateful native instances are supported.

The current standard library already lives under `stdlib/modules/` and builds
as `libama_std.a`. Keep that separation: application-facing list behavior belongs
there, while compiler support supplies the state/type machinery it depends on.

## Work plan and verification

Do not implement every part at once. Resolve a small set of complete programs
before changing the grammar.

- [ ] Specify where owned `.module.path` and reference `&.module.path`
  declarations are permitted. Define persistent internal state, typed lists,
  temporary values, and the migration from current aliases.
- [ ] Implement mandatory `&` prefixes for reference declarations and all uses,
  including reference parameters, results, named argument labels, assignments,
  and call targets. Reject mismatches between name prefixes and declared types.
- [ ] Define resolution of `&owned_instance` acquisition versus an existing
  `&reference` variable, including same-scope name collisions. Reading or copying
  a reference must not add another reference level.
- [ ] Implement typed `&instance` acquisition and reference passing without
  extending the target lifetime or exposing C addresses.
- [ ] Specify and implement the call-attached `:` alternative, failure-reason
  access, default failure behavior, and result/argument ordering.
- [ ] Implement runtime acquisition/release, busy rejection, stable active
  storage, and generation-checked identities.
- [ ] Specify owner-mediated deletion, descendant preflight, cleanup authority,
  and reference invalidation for entire owned trees.
- [ ] Write a complete two-counter example demonstrating independent state and
  explicit calls without repeated module paths in the body.
- [ ] Write a basket example with a list of item states, append requests,
  selection, stored result reads, and iteration.
- [ ] Write nested-list and shared-object examples to expose cursor and lifetime
  ambiguities. Preserve separate argument lines and avoid square brackets.
- [ ] Specify per-call defaults, retained results, cloning contents, and failure
  behavior. Confirm which old programs intentionally change meaning.
- [ ] Complete the explicit-destruction policy and reference default rules;
  distinguish temporary selections, saved references, and optional snapshots.
- [ ] Implement independent state execution before dynamic lists. Decide whether
  existing alias calls use a default instance for compatibility.
- [ ] Implement creation, explicit cloning, and cleanup of owned state trees.
- [ ] Implement `@enum`, constant access, diagnostics, and validation rules.
- [ ] Decide separately whether to rename `@Ama`; migrate all examples and
  normative grammar only after that decision.
- [ ] Define the native state ABI and compiler/library type contract.
- [ ] Implement the list module, allocation failures, and active-access checks.
- [ ] Update `README.md`, `EBNF.md`, and `CONTRIBUTORS.md` with implemented rules;
  keep unresolved proposals here rather than in the normative specification.

Verification should cover independent objects, persistent internal values,
per-call defaults, repeated identical requests, clone independence of owned
trees/lists, preserved identity of external references, and cleanup on failure.
List tests should include empty lists, growth, invalid positions, removal,
nested access, and forbidden structural changes during active calls. Reference
tests should cover mandatory name prefixes, prefix/type mismatches, reference
copying without another indirection level, name-resolution collisions, prefixed
named arguments and call targets, static type rejection, deleted targets, slot reuse, owner
destruction invalidating child references, and invalid active destruction. Verify
that missing/busy targets take `:`, failed calls do not assign outputs, body
errors do not take `:`, and busy state is released on all supported exit paths.
Deletion tests must reject busy descendants before freeing any part of a tree. Native tests must demonstrate that state is not accidentally shared
through C globals. Enum tests should reject executable bodies and mutation.

Keep the existing two test harnesses: C component tests under `tests/unit/` and
whole-program scenarios under `tests/integration/`. Use the normal recursive
Makefiles and explicit `it_*` test functions; no new testing framework is needed.
