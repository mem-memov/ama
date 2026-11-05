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
independently existing object. User-visible references, if introduced, should
identify whole typed objects and expose their public interfaces, not memory
addresses or individual internal variables.

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
- `@enum` with an empty body is the chosen direction for named constants.
- Preserve the line-oriented style: declarations and argument values are
  separate lines; arguments remain nested beneath their call.

### Proposed rules requiring confirmation

- Clone owned children recursively and preserve external object references.
- Preserve the current per-call defaults for parameters and results while
  introducing persistent internal variables.
- Give a system list an action parameter whose values come from an enum module.
- Provide typed, opaque references rather than numerical addresses.
- Use checked handles for independently addressable objects.
- Forbid structural changes to a list while an element is executing.
- Let a list select an element through a cursor-like result.
- Keep an instance per module as the default target for existing alias calls.

These are design candidates, not all accepted language rules. In particular,
the cursor design and lifetime policy still need practical examples.

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
independent state. These relationships must not be inferred merely from an
import or alias declaration.

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

Separate a temporary selection used for one call from a reference stored for
later use. The former must remain valid until the call ends. The latter needs
a defined lifetime policy.

A candidate runtime handle contains a type ID, slot index, and generation.
Access checks type, bounds, occupancy, and generation before resolving a pointer.
Deleting an object invalidates its handles even if its slot is reused. Generation
wraparound must not make an old handle valid again. An ordinary numeric value
must not be accepted as a typed reference merely because its bits match a handle.

Checked handles detect invalid access; they do not automatically reclaim leaks,
prevent unintended shared mutation, or provide thread safety.

Open lifetime choices are explicit destruction, region ownership, or tracing
garbage collection. Reference counting is another possibility but requires cycle
handling and reference updates on copies. A Rust-style ownership and borrowing
system is not the intended user-facing model: Ama should remain familiar and
small. The exact balance between runtime checks and automatic cleanup is open.

Initially, avoid exposing standalone references to owned internal children.
Otherwise parent destruction and child references need coordinated invalidation.
Also decide how to reject destruction of an executing state and conflicting
reentry into the same state. Existing module-cycle restrictions are not lifted
merely by introducing multiple instances.

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

- [ ] Agree on declaration syntax for persistent internal state, owned children,
  typed lists, temporary values, and external references.
- [ ] Write a complete two-counter example demonstrating independent state and
  explicit calls without repeated module paths in the body.
- [ ] Write a basket example with a list of item states, append requests,
  selection, stored result reads, and iteration.
- [ ] Write nested-list and shared-object examples to expose cursor and lifetime
  ambiguities. Preserve separate argument lines and avoid square brackets.
- [ ] Specify per-call defaults, retained results, cloning contents, and failure
  behavior. Confirm which old programs intentionally change meaning.
- [ ] Decide instance lifetime and invalid-reference behavior; distinguish
  temporary access from saved references.
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
tests should cover wrong types, deleted targets, slot reuse, and invalid active
destruction. Native tests must demonstrate that state is not accidentally shared
through C globals. Enum tests should reject executable bodies and mutation.

Keep the existing two test harnesses: C component tests under `tests/unit/` and
whole-program scenarios under `tests/integration/`. Use the normal recursive
Makefiles and explicit `it_*` test functions; no new testing framework is needed.
