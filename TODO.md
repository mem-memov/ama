# Ama: modules, states, objects, and lists

This document records the design discussion and the work it suggests. It is a
design roadmap, not the current language specification. `README.md` and
`EBNF.md` describe implemented behavior. Nothing in this document changes the
compiler by itself.

## The concept, briefly

A module describes both executable behavior and the structure of its state.
The module is a type. An object is one instance of that state. Many objects can
use the same module code without sharing their own variables.

This extends to basic data: numbers and byte sequences are types described by
modules, not exceptions to an otherwise object-based model. `size_t` is no
longer the planned universal language value. Small values may still be stored
inline and passed efficiently; a uniform model does not require heap allocation
for each number. Native objects expose typed Ama references, while their C
implementations may internally hold `void*` payloads.

Declarations use a name (with `&` and/or `[]`), an unmodified module path on
the next line, and optional literal initialization one space deeper. List
initialization may contain several such lines. Integer literals are not a
special expression category: literal data occurs only in declarations and is
interpreted by the declared type. No hidden integer variables are generated.

Internal C names use deterministic hashes of complete source identities. Unicode
source names remain unchanged in diagnostics and source mappings. Generated C
uses `#line` directives to associate executable code with its Ama source.

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
Both owned and reference declarations put only `.sys.obj` on the second line.
Modifiers belong to the name on the first line: `item` is owned, `&item` is a
reference, `items[]` owns a list of states, and `&items[]` owns a list of
references. Reference variable names start with `&`, as in `&target`; this is
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
- Adopt the uniform module/type model for basic data as well: numbers and byte
  sequences are fundamental types represented through modules.
- Remove `size_t` from the future language semantics as a universal value; keep
  C storage choices separate from Ama types.
- Declaration literals are interpreted by the declared module type. Remove
  integer literals as a special expression category and reject literal arguments
  or assignments outside declarations; do not synthesize hidden variables.
- Index types and conversion operations belong to the standard library; the
  compiler has an explicit mapping to the required operations.
- Generate internal C identifiers from complete source identities using hashes,
  with collision checks, preserving Unicode source identities.
- Emit `#line` mappings for source diagnostics and debugging.
- Native data crosses Ama interfaces through typed references, not bare `void*`
  values or types guessed from parameter names. Native payloads may use `void*`
  internally under an explicit type and lifecycle contract.
- Execution works over a selected state; calling does not require copying it.
- Application code controls reactions and the order of child calls.
- Repeated identical calls still execute the body repeatedly.
- Every execution call uses a mandatory trailing `!` immediately after the complete target,
  whether or not it has arguments: `worker!`, `target&!`, `items[position]!`,
  `targets&[position]!`, `items[]!`, and `targets&[]!`. Arguments keep their
  separate indented lines. Bare names do not execute modules. A standalone `^`
  exits the nearest loop (also from nested branches); `@!` remains the executable-body annotation.
- Declarations establish persistent state; ordinary calls do not recreate it.
- No separate constructor annotation or section is needed.
- Parameters and results reset to defaults before ordinary body execution, then
  retain their values until the next execution. Body variables persist.
- `>>` selects a read of saved results with no resets or body execution. Such
  reads forbid `<`, `>`, and the target suffix `!`, and expose only results.
  For example, `sensor` followed by ` measurement >> value` reads without
  executing; `sensor!` executes and cannot contain `>>`.
  Before first execution, saved results have their declared defaults. The `?`
  condition marker has no query meaning.
- Explicit state cloning is a required capability.
- Lists must support an element count unknown at compile time.
- Lists should be usable as owned parts of another module's state.
- List operations can be requested through parameters of one list module.
  Separate procedure modules for every operation are not required.
- Declaration modifiers belong only to the variable name on the first line;
  the second line contains only the module path, without `&` or brackets.
- `item` declares an owned state and `&item` a typed reference.
- `items[]` declares a dynamic list of owned states; `&items[]` declares a
  list of references, not a reference to a whole list.
- `items[index]` selects an owned element; `&items[index]` accesses a reference
  element when `&items[]` is declared. `&instance` obtains a reference to an
  owned instance.
- Empty brackets designate the whole list in declarations and collection calls.
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
- Give a system list an action parameter whose values come from an enum module.
- Use checked handles for independently addressable objects.
- Forbid structural changes to a list while an element is executing.
- Keep an instance per module as the default target for existing alias calls.

These are design candidates, not all accepted language rules. In particular,
iteration, exact failure-status syntax, and destruction API still need
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
independent state. In the proposed syntax, the name distinguishes an owned
instance (`item`) from a reference (`&item`); both use `.module.path` as the
second declaration line. This changes the meaning
of today's alias declarations and requires an explicit migration decision.

The implementation can generate a structure for each module and pass its
address explicitly to its executable function:

```c
typedef struct {
    AmaNumber increment;
    AmaNumber value;
    AmaNumber accumulated;
} CounterState;

void counter_execute(CounterState* state);
```

`AmaNumber` is a placeholder for the future numeric representation, not an
existing C type or a decision about number width. This is an implementation
sketch, not a proposed native ABI. The pointer is
an implementation detail, not an Ama value. Public parameters and results and
their reset behavior may eventually use a separate call frame, but completed
interface values must remain available for `>>` reads.

Avoid a mutable global "current instance" and avoid loading selected state into
the existing global variables before a call. Explicit contexts make nested
calls and instance independence easier to understand.

### 2. Module identity

A normalized path relative to the compilation namespace identifies a module.
The standard library already shares that namespace with project modules;
duplicate paths are rejected.

Internal generated C names must use deterministic hashes rather than replacing
slashes with underscores or requiring ASCII source identifiers. Hash the full
normalized module path as UTF-8 bytes. For a member, hash its module identity
and member name with unambiguous component boundaries. Use category prefixes
such as `ama_state_<hash>`, `ama_call_<hash>`, and `ama_field_<hash>`; reference
types, native entry points, generated parameters, header filenames, and guards
must follow the same naming service. These spellings illustrate the scheme,
not a finalized ABI.

Different source identities must never silently acquire the same C name. Keep
an identity registry, detect collisions, and issue a deterministic diagnostic
(or use an explicitly specified disambiguation scheme). Choose the hash algorithm,
width, identity encoding, and output spelling before treating names as stable.
Do not use process-randomized hashes or scan-order suffixes. Test collision
handling with a controlled fixture rather than expecting real collisions.

Preserve current case-sensitive UTF-8 byte identity. Do not silently introduce
Unicode normalization, case folding, or transliteration. ASCII-safe generated
identifiers must support international source names rather than restricting
them. Any stricter UTF-8 validation is a separate explicit language decision.

Compact integers may still index runtime type tables within one program. Those
indices are distinct from deterministic C names and are not persistent type IDs.
Neither hashes nor paths establish state-layout or native ABI compatibility;
persistence would also require schema/version information.

#### Source names and debugging

Emit line directives before relevant executable source regions, for example:

```c
#line 12 "магазин/товар"
```

Paths and line numbers must come from actual source locations. Decide how paths
are resolved relative to the compiler invocation so a debugger can find files.
Keep Unicode filename bytes and correctly escape quotes, backslashes, and any
control bytes in C strings. Restore a generated-file location for scaffolding
that has no source equivalent rather than attributing every helper error to the
last Ama line. Mapping must handle branches, loops, returns, and module changes.

`#line` supplies file/line attribution; it does not restore source variable names
or make optimized code perfectly debuggable. Provide a generated symbol map or
other tooling for hashed variable/type names. Readable source labels in generated
comments must also be escaped so source names cannot terminate a C comment.

Hash-based naming is now implemented for current states, calls, fields, native
functions/arguments, headers, and guards. The naming service uses framed UTF-8
identities with 64-bit FNV-1a and collision rejection; see `CONTRIBUTORS.md` for
the exact protocol. Maintained native implementations and fixtures are migrated.
Old native names are not ABI-compatible. Reference types and future lifecycle adapters remain later work and must use
the same service. File/line source mappings are now implemented separately.

### 3. Creation, calls, and cloning

#### Typed declarations and literal initialization

The future declaration shape is:

```text
count
.std.integer
 255

title
.std.text
 Hello, world!

values[]
.std.integer
 10
 20
 30

&selected
.shop.item
```

Module paths are illustrative until library type names are chosen. The optional
initializer is one indentation level deeper than the name and type lines. For
lists, each initial line supplies a literal for an element. A reference must not
be constructible from arbitrary literal bytes: reference initialization, if
supported, needs an explicit validated contract.

Literals are data given to the declared type, not implicit integer expressions.
Outside declarations use named values. Do not desugar a literal argument into a
hidden integer variable. The parser must distinguish declarations from calls
with indented arguments and reject duplicate declarations. Existing executable
assignment syntax must be migrated: its source will be a named value, not a
bare numeric literal. Defaults in parameter/result sections use the same typed
initialization scheme.

Literal decoding and type initialization are distinct stages. A proposed native
literal interface takes immutable bytes with an explicit length, interprets them,
and reports invalid input or range errors against the initializer's location.
Text `255` is parsed into numeric state; casting a byte pointer does not perform
this conversion. UTF-8 is the recommended format rather than host-dependent
`wchar_t`; finalize this choice and its exact decoding rules before implementation.
Specify empty strings, significant whitespace, comment characters, escapes,
multiline content, and binary encodings. Preserve the distinction between list
entries and a multiline literal for one entry.

The input may be a view of compiler-emitted immutable storage or a copied buffer,
but its lifetime must be explicit. Literal initialization runs once per created
owned instance; parameter/result defaults are reinstated according to the call
contract. A type-level initialization hook is not an Ama constructor section.
Decide whether literal errors are checked during translation, execution, or both;
C implementation availability alone does not provide compile-time evaluation.

Without an initializer, use the declared type's defined initial-state contract.
Numbers can start at zero, text empty, and lists empty. Types such as external
resources may need an empty state or require explicit initialization. Zeroing
arbitrary native bytes is not a general initialization algorithm.

#### Lifecycle

| Operation | Intended effect |
|---|---|
| Create | Initialize a fresh state and its owned children |
| Call | Supply inputs and execute against an existing state |
| Read result | Read the last stored public output without execution |
| Clone | Create a distinct copy of current owned state |
| Destroy | Release owned resources under the chosen lifetime policy |

Creation and cloning are different. A new counter starts from declared initial
values; a clone of a counter at 57 starts at 57.

There is no constructor section. Typed declarations establish initial values,
owned child declarations establish child states, and lists start empty unless
initial elements or an explicit initial length are supplied. Complex application
setup can be requested through an ordinary call; no hidden one-time body is
introduced.

The implemented numeric model now resets only parameters and results to their
declared defaults. Body variables start at zero with the module state and
persist across calls. Assignment targets and call-result receivers are collected
before reads are parsed, including targets inside conditional bodies. Reading
a variable before its first textual write observes its stored value, initially
zero. Names with no declaration or write target remain errors.

Under the currently implemented numeric syntax, every assignment executes
whenever reached; `value` followed by `0` explicitly resets it. In the future
typed syntax, an indented declaration literal initializes only once and an
assignment uses a named source. This is an intentional syntax migration. The initial
implementation retains `size_t` storage and default instances. Owned child
declarations, references, lists, and the new fundamental types remain planned.

The ordinary execution-call protocol is:

1. Select and validate the target instance.
2. Establish per-execution parameter/result defaults.
3. Copy supplied parameter values.
4. Execute the body using the selected internal state.
5. Copy requested results back and retain the resulting object/interface state.

A retained-result read uses a target without `!` and one or more `>>` requests
in its fully validated argument list. `target!` with `>>` is an error. It copies requested results without steps 2-4, never accepts
parameters or mixed `>` arrows, and uses existing defaults before first execution.
The current numeric compiler initializes default instances statically for this
contract; future type-directed instance creation must do the same semantically.

If defaults remain per-call, omitted inputs mean their defaults, not "retain
the previous input." An application that needs a previous input saves it in
internal state. Partial-update behavior must be explicit in its interface.

The storage-duration rule is settled: variables in the parameter and result
sections are temporary for a call; variables declared in the module body are
persistent instance state. No additional temporary-versus-persistent annotation
is needed. Exact declaration placement and public result access syntax remain
to be specified.

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
.sys.obj
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

The compiler derives the reference category from the `&` name prefix and the
referenced module type from the unmodified path on the second line. The older
`&.sys.obj` second-line notation is superseded. A reference cannot be accessed by dropping its prefix. Resolve
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

A list is now a language-level declaration form rather than a required user
wrapper around `sys/list`. A runtime or standard library component can still
implement its storage. The compiler knows the element type from the module
path; runtime length is independent of compile-time layout knowledge.

The declaration forms are:

```text
item
.shop.item

&item_ref
.shop.item

items[]
.shop.item

&item_refs[]
.shop.item
```

The first owns one state; the second stores one reference; the third owns
element states; the fourth owns reference slots pointing to independent objects.
Only names carry `&` and `[]`. The second line remains just a module path.
`&items[]` means a list of references, never a reference to an entire list.
References to whole collections and multidimensional lists are not yet designed.

Lists are initially empty. Element type is fixed for their lifetime. Initial
length, if later supported, creates independently initialized owned elements;
reference elements need an explicit empty-reference rule. Distinguish initial
length from reserved capacity.

Cloning an owned-state list recursively clones owned element state. Cloning
a reference list copies references to the same targets. Removing or clearing
reference slots does not destroy their targets; owned element removal releases
the element and its owned children.

Potential inputs and results, with names still provisional:

| Member | Role |
|---|---|
| Element type | Fixed by the module path in the declaration |
| Action | Selects the operation for this call |
| Index | Selects a position for operations that need one |
| Source state | Supplies a state to clone when requested |
| Length result | Reports the current element count |
| Error result | Reports invalid positions, allocation failure, or invalid use |

Useful actions include no operation, append initialized state, append clone,
remove, and clear; reference lists also need append-reference behavior. This is a candidate operation set, not a fixed API.
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

#### Indexed calls and whole-list calls

The earlier cursor design (`items.item`) is superseded by explicit indexed
selection. Brackets were reconsidered and adopted for list names and indexing.
These examples describe future syntax, not current compiler support:

```text
items[position]
 price < price
 quantity < quantity
:
 handle_unavailable
```

An owned element call executes over that element's state. A reference element
uses the same call shape, with the mandatory prefix:

```text
&item_refs[position]
 < 255
:
 handle_unavailable
```

An out-of-bounds position prevents execution and takes the alternative branch.
For a reference element, a missing or busy target also prevents execution.
Indexed selection does not modify a shared cursor, so inner selection does not
silently replace an outer call's target. Active-state protection still applies.

Whole-list operations retain empty brackets:

```text
items[]
 action < actions.append
```

Collection actions and their parameter names remain provisional. For a reference
list, the collection name retains `&`, but a whole-list call acts on the owned
collection of reference slots, not on every target. Passing a source reference
for append needs a defined signature; it does not require an action module per
operation.

Obtaining a saved reference to an owned element was sketched as
`&selected &items[position]`. This is acquisition when `items[]` is declared;
with a declared `&items[]`, `&items[position]` instead reads a reference element.
Name resolution must distinguish these cases and reject ambiguous base names.
Failure handling for reference acquisition remains open; do not conflate it
with the already defined alternative on calls. Saved element references must
preserve object identity through relocation and index shifts, and become invalid
when their element or owner is deleted.

Iteration syntax, stored-result access, length access, index origin, insertion
order, and mutation during traversal still need specification. The former
`items.length` suggestion must be reconciled with the empty-bracket whole-list
notation; it is not a settled exception. Keep each argument on its own line.

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

Use `@enum` with an empty body to explicitly mark a constant module. The
updated declaration design also applies here; the earlier untyped name/number
pairs are superseded. A proposed example is:

```text
@>
none
.std.integer
 0
append
.std.integer
 1
remove
.std.integer
 2
clear
.std.integer
 3
@enum
```

Proposed validation rules: only result declarations with explicit typed constant
initializers; no parameters, callable aliases, executable body, or mutation.
Initially use the library integer type for actions; requirements for other
constant-capable types and compile-time validation still need definition. Enum members are
available without executing the module, and the enum itself is not callable.

Initially, members can be named numeric constants rather than a new closed
enum type. Their values follow the future number-type contract rather than
implicitly being `size_t`. An action parameter receives the member's numeric value,
not the enum module itself. Unknown action values must produce a defined error.
Using zero for a no-operation default is convenient but remains an API choice.
Member access syntax, duplicate-value policy, and enum type checking remain open.

`@enum` must not change unfinished declaration files into valid modules by
accident: the selector is explicit. Empty bodies still require their selector.

The suggestion to replace `@Ama` with `@do` would emphasize execution rather
than implementation language. Other candidates were `@body`, `@proc`, and
`@code`. No replacement is accepted yet. `@C` remains the current native selector.

### 10. Uniform types instead of a universal `size_t`

The chosen direction is the radical uniform model: modules describe data types
as well as behavior. Owned states, typed references, and lists remain the
common composition mechanisms. Numbers and byte sequences are fundamental
module-defined types with suitable native representations.

Do not replace the universal `size_t` with an equally universal byte or untyped
byte array. A byte sequence can hold text or binary data, but code should not
need to reconstruct a multi-byte encoding to increment a list index. Nor should
it guess whether a buffer contains a number, reference, or serialized object.

Language-level meaning must be distinct from representation:

| Meaning | Possible implementation; not a settled ABI |
|---|---|
| Numeric state/value | Inline fixed-width integer or managed numeric payload |
| Byte sequence | Buffer and length with defined ownership |
| Owned module state | Generated structure with owned children |
| Reference to a module instance | Checked typed handle |
| List | Runtime storage of typed owned states or reference slots |
| Opaque native state | Typed object whose implementation holds a C payload |

An owned basic value must have value semantics under the chosen copy/clone
rules; sharing its representation must not introduce surprising shared mutable
state. Optimizing small values inline is allowed. These optimizations must not
change observable ownership, identity, or reference behavior.

Number width, signedness, range, overflow, and conversions belong to the selected
standard library numeric type, not a privileged integer token in the language.
The library defines the index type and checked index-conversion operations. The
compiler keeps an explicit mapping to those operations for indexed access and
other structural needs. Validate the mapped signatures and fail clearly if the
required library contract is missing. Do not infer it from parameter names.
The runtime still uses host indices internally only after range validation.

Conditions similarly need a documented library operation or protocol to obtain
a truth value; removing built-in integers does not eliminate this structural
requirement. Paths and versioning of these contracts remain open. The compiler
must not quietly reinstate universal `size_t` through special-case arithmetic.

Literals exist only as declaration initializers and are interpreted by their
explicit module type. No integer expression literals or automatically generated
integer temporaries are planned. Fundamental modules and generated code can
still optimize numbers inline; users need not allocate one heap object per
arithmetic operation. Define value copy semantics independently of storage.

`size_t` remains useful inside C for allocation sizes and host indexing. It is
not an Ama type definition. Cross-boundary conversions must check range rather
than truncate silently, especially on different host word sizes.

### 11. Native C data and the standard library boundary

The compiler must know type identities, layouts or opaque-state contracts,
instance selection, and generated initialization, cloning, execution, and
cleanup operations. Current numeric `@C` signatures cannot express this model.
The new native ABI must be designed explicitly; it is not implied by an existing
`size_t` parameter or result.

#### Module-to-module data transfer

An ordinary parameter receives an independent value of its declared module type.
A reference parameter receives a checked reference to the original instance.
Type compatibility is established by full module identity, not by argument name
or by the fact that both payloads happen to fit in a `void*`.

For an ordinary composite value, copying clones owned children and lists and
preserves external reference targets. Reject unsupported copies rather than
silently aliasing an owning payload. Implementations may elide physical copies
or share immutable backing storage only if observable value semantics remain
unchanged. A C pointer to a value parameter denotes temporary access to its
prepared parameter storage, not a retained Ama reference.

Results by value replace destination values under the type's copy/cleanup
contract. Define preparation and commit ordering so a failed operation does not
leave a partially replaced destination. Reference results copy handles without
transferring ownership. Mutable versus read-only access to copied C parameters
and the technical function return status are ABI details still to finalize.

The compiler generates state types, reference declarations, function prototypes,
and lifecycle adapters from complete module interfaces. Native fundamental types
supply their C layout or opaque payload contract; the compiler composes those
into user module states. Handwritten native code includes generated headers
rather than reproducing structures. Use the hash-based naming scheme for every
such generated type and symbol, including Unicode source names.

A C implementation may store a `void*` payload inside its native state. Ama sees
a typed instance and checked references, never a bare pointer. For example,
a future consumer of an opaque image declares:

```text
@<
&image
.native.image
```

`native/image` is an illustrative type path, not an implemented module. Its
implementation knows the actual C image structure. Another native module may
work directly with the same image without copying its buffer, but must use the
agreed type interface to obtain temporary payload access.

Parameter names describe roles; they do not establish type identity. Two native
implementations calling different layouts `image` are not compatible. Renaming
a parameter must not change its type. Require agreement on the module-defined
type and a shared native accessor/header contract rather than independently
casting a `void*` based on its name.

The native type owner must specify:

- How a fresh payload is initialized and who owns its allocations.
- How temporary access is acquired and released, including busy checks.
- Whether clone creates an independent resource, or is unsupported.
- How owned payloads are destroyed exactly once.
- How external or borrowed C resources are represented without assuming ownership.
- What happens on initialization, clone, or cleanup failure.

A file, socket, image buffer, or third-party context cannot be cloned by blindly
copying its pointer. A type that cannot clone needs a defined rejection and
failure propagation when contained in an owned tree. Do not silently share its
payload under a clone operation advertised as independent.

Opaque native types still have trusted C implementations. Typed wrappers cannot
make arbitrary incorrect C casts or saved raw pointers memory-safe. Require
native modules to use the declared contract and forbid retaining temporary
state/payload pointers after access ends. Any persistent cross-module link must
use a supported lifetime contract rather than an untracked borrowed pointer.

Native globals and C `static` variables are not automatically instance state.
They must not accidentally share per-object data. Busy state and cleanup are
managed through the runtime/native contract, not bypassed by direct payload use.
Whether resource finalization needs a native callback is an ABI question; it
does not introduce an Ama constructor section.

The standard library can implement allocation, fundamental native data types,
list growth, and handle tables using compiler-provided descriptions or generated
adapters. Static specialization versus runtime descriptors remains open. Keep
the existing separation: library sources live under `stdlib/modules/` and build
as `libama_std.a`; the compiler supplies type/state machinery. A module used as
a type in a declaration does not automatically become an ordinary runtime value
that can be passed to every function.

## Work plan and verification

Do not implement every part at once. Resolve a small set of complete programs
before changing the grammar.

### Stackless execution: binding requirement and immediate priority

The accepted execution model is now normative in `EBNF.md`:

- Ordinary Ama modules become labeled blocks in one generated C execution
  function. Calls and returns are direct label transfers, not C function calls.
- Multiple call sites use explicit continuation selectors and return dispatch.
  Keep control storage per active instance, separate from persistent values and
  parameter/result data. Do not introduce an activation stack, including a
  disguised heap stack or dynamic chain of continuations.
- `@C` function code must be mandatorily inlined. Apply the same constraint to
  native helpers and generated lifecycle operations; plain `inline` or LTO is
  not evidence that a required call disappeared.
- Reject recursive calls, active-instance reentry, and unsupported native code
  that would leave a required C call. Allocation/I/O and external libraries have
  no implicit exception and require an explicit compatible implementation plan.
- This constraint applies to generated program execution, not to the compiler
  implementation. Hardware stack spills and compiler-generated helper calls need
  target-specific verification beyond source-level C control-flow checks.

Current numeric module functions and typed callback-based invocation are
nonconforming transitional implementations. Completed checkboxes below record
what exists; they do not certify compliance with stackless execution. Preserve
validated declarations, identities, source locations, layouts, explicit data
storage, and ownership/failure rules. Replace the call-based execution strategy
before extending typed executable-body generation on top of it.

Immediate work, in order:

1. [ ] Audit generated numeric execution, typed call adapters, lifecycle callback
   dispatch, and native/helper/library calls. Classify reusable data contracts
   separately from execution mechanisms that must be replaced.
2. [ ] Define a small execution representation for module entry labels, call-site
   continuation labels/selectors, exit dispatch, early returns, and failure paths.
   Specify static validation for recursion and active-instance reentry, without
   storing a dynamic activation chain.
3. [ ] Generate ordinary Ama modules as labeled blocks in one C execution
   function. Preserve per-call defaults, persistent state, and retained-result
   reads through explicit data movement and label transfers.
4. [ ] Choose and implement mandatory `@C` inlining for the supported toolchain.
   Specify native code eligibility and diagnostics for unsupported cases. Resolve
   allocation, I/O, and transitive external calls without silently granting them
   an exception. Reject code when the required inlining cannot be guaranteed.
5. [ ] Specialize initialization, copy, move, and destruction for known types so
   generated execution does not call descriptor function pointers. Retain type
   descriptors as metadata where useful, without runtime indirect dispatch.
6. [ ] Test shared module bodies with multiple callers, nested acyclic calls,
   continuation selection, early exits, native errors, and retained reads. Reject
   recursion and reentry. Compile under supported optimization settings and verify
   that required native/helper calls do not survive; keep machine-code stack
   verification distinct from checking generated C module control flow.
7. [ ] Resume typed body binding and executable project integration using this
   execution model, then migrate and retire transitional call-based backends.

### Original migration sequence (progress reference)

The immediate stackless work above takes precedence over this original sequence.
The following stages retain the broader migration context and completed progress;
they must not be used to extend the transitional call-based backend.
The target syntax is now recorded in `EBNF.md`;
it deliberately precedes implementation. Earlier exploratory syntax elsewhere
in this design log must not override that revision. In particular, use suffix
reference names (`target&`, `targets&[]`), prefix acquisition (`&item`), `|`
line literals, `@!`, `@=`, and trailing `!` for execution calls only; do not
reintroduce `*` or `()`.

1. **Shared identities and deterministic internal C names.** Establish full
   module/member identities, a single hash-based naming service, collision
   checks, and original Unicode labels for diagnostics. Migrate generated types,
   fields, functions, native interfaces, and headers together. Finish when all
   existing tests pass, Unicode native names work, `shop/item` and `shop_item`
   are distinct, and controlled collisions are rejected. This foundational
   task did not require new syntax or a new value ABI.
2. **Source locations and mappings.** Preserve source path, line, and column
   information at the parsing boundaries so later syntax changes keep useful
   diagnostics. Emit escaped `#line` directives for executable regions, retain
   Unicode filenames, and restore generated locations for scaffolding. Verify
   compiler diagnostic locations and source-level debugger attribution; document
   optimization limits and the separate need to map hashed variable names.
3. **An intermediate program representation.** Separate parsing, semantic
   validation, and C emission. Represent declarations, assignments, calls,
   branches, loops, breaks, and returns explicitly with source locations and
   entity identities. First preserve implemented behavior and the existing
   native ABI; migrate generator-dependent tests alongside the new passes.
   Finish when parsing can build a program without emitting C and code generation
   consumes a validated representation rather than the token stream. Avoid a
   generic optimization framework: retain only structures needed for the language.
4. **Target declarations and literal syntax.** Parse explicit module-typed
   declarations, suffix reference/list forms, indented named-value assignments,
   and `|` initialization lines. Introduce `@!`, `@=`, and mandatory `!` on
   execution targets only; recognize retained reads from an unmarked target and
   indented `>>` requests, without consulting alias declarations. Remove special integer
   expression literals and implicit numeric variable creation; do not synthesize
   hidden integer variables. Verify whitespace, payload preservation, declaration
   versus call disambiguation, constant-module restrictions, and migration errors.
   Use the intermediate representation to keep grammar changes separate from
   runtime representation work; syntax support alone is not executable support.
5. **Module-defined values and the native contract.** Define layout, creation,
   initialization from literal bytes, copy, and cleanup operations. Implement the
   standard library number type and compiler mappings to library index/condition
   contracts. Remove universal `size_t` assumptions without exposing untyped
   pointers to Ama. Verify ownership, native signatures, range checks, unsupported
   copies, and cleanup on failure before migrating arithmetic programs fully.
6. **Owned instances and checked references.** Build recursive owned states,
   prohibit infinite fixed-size ownership, and implement prefix acquisition and
   suffix-reference uses. Add generation-checked identity, per-instance busy
   acquisition/release, owner-mediated destruction, and call alternatives `:`.
   Verify independent state, no references to references, invalidation, and refusal
   before execution rather than partial result writes. Preserve existing numeric
   state separation and persistence already implemented below.
7. **Dynamic lists.** Build owned-state and reference lists on the established
   type/lifecycle mechanisms. Implement indexed calls, collection operations,
   growth, clone/remove distinctions, and active-element protection. Verify bounds,
   nested access, stable identity, failure cleanup, and that deleting a reference
   slot does not destroy its target. Finalize traversal and length interfaces
   through complete readable examples rather than incidental parser shortcuts.

The highest-value architectural change after naming and source locations is
stage 3: the intermediate representation removes today's coupling between token
parsing and immediate C output. It lets subsequent syntax and data-model changes
be developed and tested separately. Do not combine all stages into one rewrite.
Update contributor guidance, implemented-language documentation, and relevant
unit/integration tests at each stage; never claim planned behavior is implemented.

### First task: deterministic internal naming (implemented)

Start with a shared identity-to-C-name component before changing the value ABI.
It is needed by generated state types, native bindings, typed values, references,
and international source names, and can be verified independently of new syntax.
This stage and file/line source mapping are implemented. The AST architecture stage is now implemented for the current language. Future generated entities
must use the same component.

- [x] Define canonical identity tuples, deterministic hash algorithm/width,
  ASCII spelling, category prefixes, and collision behavior.
- [x] Implement the shared naming component and a collision registry. Test Unicode,
  case differences, component boundaries, `shop/item` versus `shop_item`, scan-order
  independence, and controlled collisions.
- [x] Migrate state types, calls, fields, native signatures, generated parameter
  names, header files, and guards together; update handwritten library/native
  bindings and affected tests without overwriting contributor implementations.
- [x] Remove ASCII-only native-name restrictions that hashing makes unnecessary.
  Preserve existing source identity rules and readable diagnostics.
- [x] Complete the existing full test tree and add native Unicode integration
  coverage before considering the naming migration finished.

After naming, implement source mapping as stage 2, then introduce the intermediate
representation in stage 3 before changing declaration syntax or value layout.

- [x] Retain source paths, lines, and columns on lexer tokens and declarations/
  body write targets. Paths are borrowed from project-owned absolute resolved
  source paths; columns count UTF-8 bytes from 1. Future IR must preserve them.
- [x] Emit escaped Unicode `#line` mappings for executable operations, including
  delayed calls, parameter/result copies, branches, loops, returns, and breaks.
  Restore virtual `<ama-generated>` line 1 for synthetic scaffolding. Verify
  downstream C error locations, Unicode, filename escaping, CRLF, and byte columns.
- [ ] Verify interactive debugger stepping separately where debugger execution
  is available; document optimization limits and source-path relocation. Line
  mappings do not restore original variable names or source columns.
- [x] Introduce explicit declaration/statement/call/control nodes and separate
  parsing, semantic validation, and C emission while preserving current behavior
  and the native ABI. One AST carries raw spellings/locations and resolved IDs.
- [x] Replace immediate parser emission with generation from validated AST nodes.
  Copy token spellings, borrow project-owned source paths, preserve argument order
  and `#line` locations, use stable module-owned allocations and iterative cleanup,
  and bound active block nesting at 128 levels (including the module body).
- [x] Implement the earlier syntax with `!` on all calls, with arguments or
  without. This historical behavior is superseded for retained reads by the
  target rule below; the current parser still needs migration.
- [x] Implement the call suffix in the lexer/parser, remove declaration-based
  classification, migrate fixtures/examples, preserve argument/result ordering,
  and replace standalone `!` breaks with `^`. The numeric ABI and assignment
  syntax remain unchanged; typed/reference/list syntax is still future work.
- [x] Test current zero-argument and argument-bearing calls, Unicode/CRLF
  adjacency, separate argument lines, bare/standalone marker rejection, nested
  loop exits, and source line mappings.
- [ ] Extend call-marker tests to reference, indexed, and whole-list forms, `@!`,
  and `|` literal payload containing `!` or `^` when those features land.
- [x] Reject `^` outside loops during semantic validation and reject standalone
  `!` statements. A callee cannot inherit its caller's loop context. Preserve
  early return `>` as a separate operation.

### Retained-result reads (implemented for the numeric model)

- [x] Tokenize contiguous `>>` distinctly from `>` and `> >`; preserve source
  positions, CRLF handling, and errors encountered during arrow lookahead.
- [x] Preserve all three arrow kinds in AST, determine the entire call mode during
  validation, and reject `<` or `>` in read calls regardless of order.
- [x] Emit only result copies for reads, including native modules and partial
  named reads. Preserve ordinary execution/reset semantics and `#line` mappings.
- [x] Initialize current default instances with declared interface values before
  first execution; retain those slots after return, including early return.
- [x] Test repeated reads, positional/named rules, initial and retained values,
  native side effects, mixed arrows, and ordinary calls following reads.

### Call markers: execution `!`, retained reads without `!` (target rule)

- [x] Update `EBNF.md`: `target!` executes; `target` with one or more indented
  `>>` requests reads saved results. `!` draws attention to execution and possible
  target-state changes. Reading still writes caller-side receiver values.
- [ ] Split syntax recognition into marked execution and unmarked retained read.
  Disambiguate the latter from declarations and assignments by its indented
  `>>` requests, without resolving module aliases during parsing.
- [ ] Reject `target!` with any `>>`, unmarked targets without read requests,
  and unmarked reads containing `<` or `>`. Validate the whole argument list
  before any reset or execution; do not silently change execution into reading.
- [ ] Preserve named/positional consistency, saved defaults before first execution,
  result-only visibility, checked target access, and the optional `:` alternative.
  Reading must not jump into the target body or reset target storage.
- [ ] Migrate relevant lexer/parser/semantic fixtures and implemented-language
  documentation when the code changes. Cover owned/reference, indexed/list,
  Unicode and CRLF targets, no-argument execution, mixed arrows in both orders,
  `!`-marked read rejection, and bare target rejection.
- [ ] Carry this distinction into the stackless execution representation: only
  execution calls prepare an invocation and transfer control to an entry label;
  retained reads copy stored results without a target-body transfer.

The new syntax is specified, not implemented. Existing numeric read fixtures
still use `!`; keep their passing status distinct from target syntax compliance.

### Typed declaration frontend (implemented separately from the CLI)

- [x] Parse all four declaration storage forms with an unmodified module path,
  optional scalar initialization, and multiple list element initializers.
- [x] Preserve raw line-literal bytes and lengths in independently owned AST
  storage; support empty and long payloads without the token spelling limit.
- [x] Check nominal module identities through a resolver callback, earlier source
  order, scalar/list element compatibility, and prefix reference acquisition.
  Reject reference-to-reference acquisition and duplicate base names across
  storage forms in this initial checker.
- [x] Parse explicit body declarations and indented named assignments in target
  mode; reject numeric and line literals outside declaration initialization.
- [x] Keep declaration validation separate from executable AST validation.
  Reject typed declarations in the numeric binder/backend rather than emitting
  them with an incorrect `size_t` layout.

The frontend is ready for connection to the lifecycle ABI described below.
Typed layouts and native literal initialization are available through the
project layout export command; typed executable programs remain pending.
The frontend does not execute literals or check arbitrary typed assignments and
calls. CLI migration, implicit-local removal, library type modules, and full
reference/list semantics remain pending. Old numeric fixtures still run.

### Value lifecycle ABI (first C runtime layer implemented)

- [x] Introduce versioned canonical type descriptors with size/alignment and
  explicit initialization, byte-and-length literal, independent copy, move,
  and destruction callbacks. See `VALUE_ABI.md` for ownership and failure rules.
- [x] Support caller-owned inline state storage; do not require a heap object for
  each scalar. Reject unsupported alignments and descriptor mismatches explicitly.
- [x] Implement replacement/reset through prepare-and-commit so allocation,
  literal, or clone failure preserves the live destination.
- [x] Compose owned record fields with checked offsets and reverse-order unwind
  after partial initialization/copy failure. Require nonfailing move/cleanup.
- [x] Publish runtime headers and operations through the standard library build;
  test inline scalars, nested native resources, clone refusal, and address repair.

### Typed C layouts and descriptors (implemented through a C API)

- [x] Generate permanent body-state structures and separate parameter/result
  frame structures from validated typed declarations. Use hash-based identifiers
  and preserve Unicode source locations with `#line` directives.
- [x] Order state layouts by owned dependencies; reject recursive inline
  ownership before writing output. Allow self-typed parameters/results because
  frames do not form part of permanent state layouts.
- [x] Require explicit native layout providers and emit canonical external
  descriptors, C layout expressions, and owned record metadata.
- [x] Apply literal and earlier same-structure named initializers in declaration
  order, with reverse-order cleanup after failure. Reject cross-frame/state
  initialization, references, and lists until their contracts are defined.
- [x] Compile and run generated C across multiple translation units. Verify nested
  resources, clone/reset failure, Unicode, identical leaf names in different
  directories, empty states, and descriptor identity.

### Project typed layout export (implemented)

- [x] Add `--generate-type-layouts <root> <output-directory>` and repeatable C APIs
  connecting project scanning, typed parsing, type resolution, and layout export.
  Publish `ama_types.h` and `ama_types.c` after successful validation.
- [x] Reuse the current section selectors and draft rules. Reject missing/draft
  types and require native `.c` plus explicit `.h` layout providers.
- [x] Support an explicit typed library in the C API without implicitly parsing
  the existing numeric library as typed declarations. Keep CLI export project-only.
- [x] Preserve existing output on parse/type/layout failure; reject aliases of
  source files and of the other output. Test recovery and repeated exports in
  one process, plus compilation/execution of a real exported project.

### Typed native invocation (implemented for C callers)

- [x] Emit native interface frames, canonical frame descriptors, typed native
  prototypes, inline bridges, call schemas, and invocation adapters in type export.
  Keep typed native symbols separate from the numeric ABI.
- [x] Prepare fresh defaulted frames, independently copy explicit input bindings,
  and expose parameter/result slots with typed pointers. Reject invalid positions,
  duplicates, type mismatches, and overlapping state/frame storage.
- [x] Commit the prepared frame only on native success. Discard failed frames
  without replacing previous retained results; document that persistent native
  state and I/O are not rolled back.
- [x] Read individual retained results without execution/reset, including defaults
  before the first call. Preserve a read destination if its copy fails.
- [x] Test preparation/execution failure, omitted inputs, saved reads, zero-argument
  native functions, and C compiler rejection of mismatching native definitions.

Next: follow the immediate stackless-execution work above before extending
executable body generation. These C-call adapters are a tested ownership/retention
prototype, not the target Ama execution backend. Reuse their data and failure
contracts while replacing invocation and lifecycle callback dispatch. Grouped
result delivery and source-located failure propagation remain pending, as do
references, lists, object lifetime handles, and busy-state tracking. Current
numeric executable compilation also requires migration to labels and jumps.

### Following type and syntax tasks

- [ ] Integrate typed declaration initialization and parameter/result defaults
  with generated execution. Remove implicit numeric local creation when the
  CLI migrates to the new syntax. Frontend checks are implemented above.
- [ ] Connect the implemented byte-and-length literal callbacks to generated
  initialization, source-located errors, and execution timing. Finalize library
  text encoding rules without quoting, escaping, or multiline values.
- [ ] Define explicit compiler mappings to library index and condition operations,
  including signature validation and range/error handling.
- [ ] Extend the implemented owned-value native protocol to checked references
  and grouped result receivers, including ownership and replacement on failure.
- [ ] Define the initial fundamental module types for numbers and byte sequences,
  their language-level declaration initialization, condition behavior, numeric
  range, overflow, conversions, and value versus reference passing. Keep unresolved representation
  choices explicit.
- [ ] Replace assumptions of universal `size_t` in symbols, parsing, codegen,
  arithmetic interfaces, enum values, and list indices with the chosen type model.
  Plan migration of existing programs and native interfaces.
- [x] Generate inline owned state layouts using the lifecycle storage contract.
  Integrating them with executable CLI programs remains pending.
- [ ] Connect native state wrappers to checked temporary payload access and
  object lifetime/busy tracking. Lifecycle callbacks, unsupported clone status,
  and partial-failure cleanup are implemented in the C runtime.
- [ ] Implement declaration forms `item`, `item&`, `items[]`, and `items&[]`,
  always followed by an unmodified `.module.path`. Define permitted declaration
  sections, persistent state, temporary values, and migration from current aliases.
- [ ] Implement indexed call targets and empty-bracket collection calls with
  mandatory reference-name suffixes and checked bounds.
- [ ] Specify reference-list insertion, empty slots, clone/removal semantics,
  and saved element identity through growth and index shifts.
- [ ] Implement mandatory `&` suffixes for reference declarations and uses,
  including parameters, results, named argument labels, assignments, and call
  targets. Derive storage form from the name; reject modifiers on the type line.
- [ ] Distinguish prefix `&owned_instance` acquisition from suffix `reference&`
  access. Reject references to references; define uniqueness across storage forms.
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
- [ ] Write nested-list and shared-object examples using indexed calls, owned
  lists, and reference lists. Preserve separate argument lines and test access
  lifetime without introducing a mutable selection cursor.
- [ ] Specify per-call defaults, retained results, cloning contents, and failure
  behavior. Confirm which old programs intentionally change meaning.
- [ ] Complete the explicit-destruction policy and reference default rules;
  distinguish temporary selections, saved references, and optional snapshots.
- [x] Separate generated module code from numeric state: functions take an
  explicit state pointer, including native wrappers. Existing calls use default
  callee instances. The initial refactor preserved all resets; the following
  completed step changes body-variable lifetime. A C-driver integration test
  executes one generated module over two independent states. This is an internal
  refactor, not yet user-visible objects or persistent body variables.
- [x] Preserve numeric body variables between calls, initialize them once to zero,
  collect write targets before reads, and retain per-call defaults for parameters
  and results. Explicit assignments still execute on every reached call.
- [ ] Implement language-visible instance selection, owned child creation, and
  recursive zero/empty initialization. Reject infinite fixed-size ownership
  cycles while allowing future recursive structures through lists/references.
- [ ] Implement creation, explicit cloning, and cleanup of owned state trees.
- [ ] Implement `@=` constant modules and `@!` executable bodies; migrate
  examples, tests, section splitting, diagnostics, and contributor instructions.
  The old `@enum` proposal and implemented `@Ama` spelling are superseded.
- [ ] Define the native state ABI and compiler/library type contract.
- [ ] Implement language-level list support backed by runtime/library storage,
  allocation failures, and active-access checks.
- [ ] Update `README.md`, `EBNF.md`, and `CONTRIBUTORS.md` with implemented rules;
  keep unresolved proposals here rather than in the normative specification.

Verification should cover independent objects, persistent internal values,
per-call defaults, repeated identical requests, clone independence of owned
trees/lists, preserved identity of external references, and cleanup on failure.
List tests should include all four declaration forms, rejection of second-line
modifiers, whole-list versus element calls, owned-list clone independence,
reference-list shared targets, non-destruction of targets on reference removal,
empty lists, growth, invalid positions, removal,
nested access, and forbidden structural changes during active calls. Reference
tests should cover mandatory name prefixes, prefix/type mismatches, reference
copying without another indirection level, name-resolution collisions, prefixed
named arguments and call targets, static type rejection, deleted targets, slot reuse, owner
destruction invalidating child references, and invalid active destruction. Verify
that missing/busy targets take `:`, failed calls do not assign outputs, body
errors do not take `:`, and busy state is released on all supported exit paths.
Deletion tests must reject busy descendants before freeing any part of a tree. Native tests must demonstrate that state is not accidentally shared
through C globals. Cover typed opaque payload exchange without copying, rejection
of incompatible types regardless of parameter names, lifetime invalidation,
independent resource clones or explicit clone refusal, and exactly-once cleanup.
Numeric tests must cover declaration literals and rejection of literals in calls
and assignments (without hidden variables), value copies, range/overflow policy, index
conversion, host-size boundaries, and byte-sequence ownership. Enum tests should
reject executable bodies and mutation and follow the new numeric contract.

Keep the existing two test harnesses: C component tests under `tests/unit/` and
whole-program scenarios under `tests/integration/`. Use the normal recursive
Makefiles and explicit `it_*` test functions; no new testing framework is needed.
