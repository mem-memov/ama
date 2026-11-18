# Native ABI, modules, and lifecycle

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: native code supplies a typed foundation](#explanation-native-code-supplies-a-typed-foundation)
- [Reading the ABI contract](#reading-the-abi-contract)
- [Qualify handwritten native symbols](#qualify-handwritten-native-symbols)
- [Native modules and representations](#native-modules-and-representations)
- [Symbolic protocol registration in `.abi`](#symbolic-protocol-registration-in-abi)
- [Complete lifecycle and cleanup protocol](#complete-lifecycle-and-cleanup-protocol)

<!-- /chapter-contents -->

## Explanation: native code supplies a typed foundation

A native module is an Ama interface backed by C. The interface tells Ama what
types can be supplied and returned; the native contract explains how those
values are represented and how their resources are created, cloned, and released.
The compiler checks declarations and generated uses. It does not prove the C
implementation correct.

The most important ownership boundary is simple: generated Ama fields hold Ama
objects and references; opaque native payloads hold native resources. A checked
control resource for a separately hosted library context has the narrow exception
described in CABI-05 and [EMBED-07](c-library.md#embed-07); it is not a reference
to an object in the enclosing Ama graph. Keeping
that boundary visible lets the runtime copy the ownership graph and invalidate
references without asking arbitrary C payloads to reveal hidden pointers.

A file operation illustrates the distinction. The native payload may own a file
descriptor and its buffers. An Ama field can refer to a task that uses that
operation. A native callback may retain its own operation record, but it cannot
retain a borrowed pointer to the task or later execute Ama from that callback.
See [asynchronous event delivery](../language/async-and-parallelism.md).

## Reading the ABI contract

**Explanation.** ABI means *application binary interface*. In Ama it names the
contract connecting generated code, authored C companions, and native resources
for a supported target. The contract includes the C declarations and the
ownership, lifetime, status, and execution rules needed to use them correctly.
A matching C prototype alone cannot establish that a borrowed pointer may be
retained or that a native resource can be cloned.

This contract belongs to the product specification. Native authors need to know
what they may depend on before they write their companions; the backend needs
the same obligations before it generates declarations and adapters. Three
levels must remain distinct:

| Level | What it determines | Canonical requirements |
|---|---|---|
| Required native boundary | Nominal types, input/output direction, ownership and bounded borrows, native protocols, status channels, construction/clone/destruction, and permitted execution | CABI-01 through CABI-05, PROTO-01 through PROTO-11, LIFE-01 through LIFE-05; ERROR-01 through ERROR-06 and STACK-01 through STACK-04 |
| Concrete target binding | The exact checked declarations for the selected backend, actual authored payload types, native layout/alignment, C calling convention, and documented toolchain settings | CABI-04 and CABI-06, subject to the fixed constraints above |
| Internal realization | Private pool records, directory/free-list storage, control-field encoding, acquisition bookkeeping, and generated helper decomposition | The runtime/compiler chapters; these are not automatically public native ABI |

The implementation chapters still constrain internal realization. For example,
stable observable addresses and stale-reference invalidation remain required.
Calling a representation private does not waive those rules; it means a native
companion cannot depend on an otherwise unspecified struct offset or control
field. [RT-02](runtime.md#rt-02) and [GEN-03](compiler.md#gen-03) permit proved
layout optimizations without changing the allowed native observations.

### A generated header expresses the contract for one build

**Explanation.** The source of requirements is this specification together with
the validated Ama interface and authored native representation. A generated
header expresses those inputs as exact C declarations for a particular target
and configuration. Native code uses that header instead of copying declarations
from prose or reconstructing them from generated names.

The header is binding when compiling that build, but is not an independent
source of language requirements. If it disagrees with a requirement, fix the
generator or its inputs; do not amend the specification to match accidental
output. The checks in CABI-06 connect the specification, independently usable
authoring header, and final project assembly.

Independent header checking does not make each native Ama module an independently
linkable binary or establish a universal plugin ABI. [GEN-01](compiler.md#gen-01)
requires one C assembly per compiled Ama component, and
[STACK-01](compiler.md#stack-01) still requires the native entry boundary within
that assembly to disappear through verified inlining. A complete independently
compiled component can instead expose the generated public C library interface
under [EMBED-01](c-library.md#embed-01).
Its public host header is distinct from an individual native companion's
authoring header. Ordinary native helpers and external C libraries, including
that separate component's explicit host interface, may use normal calls under
[STACK-02](compiler.md#stack-02).

### Which names and layouts in examples are fixed?

**Reading convention.** A C block illustrates only the roles and constraints
identified by its surrounding requirement. Placeholder spellings do not create
additional required symbols. In particular:

| Spelling or representation | Status |
|---|---|
| `AMA_NATIVE(local_name)` | Required module-qualification facility under NAME-05 |
| `AmaStatus` and its `code`, `message`, `message_length` fields | Required shared status declaration under ERROR-03; the physical layout follows the selected target C ABI |
| The six `.abi` symbols | Fixed binding syntax under PROTO-01/PROTO-11; authored hook names are selected by their records |
| `from_size` for the exact module `..integer` | Fixed qualified helper name and typed bridge under PROTO-03 |
| `State`, `initialize`, `to_condition`, `to_index`, and C parameter names in examples | Illustrative local names unless the particular rule explicitly fixes one; actual hooks come from `.abi` and actual payload types from the native representation |
| Private generated structs, field offsets, selectors, and helper names | No public spelling/layout guarantee unless explicitly exposed by the supported native contract |

Illustrative names do not make the associated types, constness, data direction,
fresh-storage requirements, or status behavior optional. Conversely, the status
declaration does not prescribe a universal byte size, padding, or pointer width:
these depend on the target described under CABI-06. An in-memory C struct is not
by itself a serialization format for files or cross-context messages.

## Qualify handwritten native symbols

<a id="name-05"></a>

**NAME-05.** The generated assembly MUST also provide a module-qualified naming
facility for handwritten native C. In a companion's generated inclusion context,
`AMA_NATIVE(local_name)` denotes a C identifier formed from a collision-checked
module prefix and that local C identifier. The module prefix is derived from the
full logical identity, including the root kind, in a separate native-symbol
category. Prefix encoding and
concatenation must be injective over the accepted module/local-name pairs.
Identical local names in different module companions therefore remain distinct.

Native authors use this facility for module-owned file-scope functions, types,
tags, enumeration constants and objects that require isolation. The `.abi`
identifier names a function in this module context, for example:

```c
/* Included in the declaring module's generated naming context. */
static AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state);
```

```text
+ initialize
```

This record binds that module's `AMA_NATIVE(initialize)`, not a global function
selected by basename. The generator establishes the same prefix for its `.h`
and `.c` and generated typed uses, and removes the temporary naming context
after each inclusion. The prefix is independent of file discovery/include order.
Shared ABI declarations such as `AmaStatus` have one documented runtime namespace.

That namespace and module-prefix isolation apply within one compiled component.
When several compiled Ama libraries are linked together, NAME-01 and
[EMBED-06](c-library.md#embed-06) additionally require component-qualified external
symbols or genuine internal C linkage, including native helpers. Shared-library
visibility alone does not replace unique external names when static objects are
linked together. The public library host header uses its own component-qualified
types; it must not export
an unqualified private `AmaStatus`, native payload type, or authoring macro merely
because it is present in the component's native assembly. A host status type
expresses the documented host contract rather than exposing that private native
declaration by accident.

This mechanism qualifies authored identifiers; it does not parse or rewrite
arbitrary C. The preprocessor's macro namespace is still shared: companion macros
and include guards must be uniquely named or contained/undefined by their author;
`AMA_NATIVE(...)` is not a macro-name rewriting mechanism for `#define`/`#ifdef`.
External C dependencies retain their public names and ordinary C linkage rules.
Unqualified conflicting C definitions, leaked macros or incompatible declarations
are build errors, never permission to bind one module's hook to another's.

## Native modules and representations

### Connect the declared interface to C

<a id="cabi-01"></a>

**CABI-01.** `@C` selects the companion C implementation. Its declared runtime Ama
parameters/results define the interface; the generator supplies collision-safe
C types and declarations. C parameters receive the corresponding C types;
results are written through typed output storage. A native implementation has
no Ama-visible C return value. Exact generated adapter declarations must be
documented and checked under CABI-06; where this chapter leaves their spelling
open, that choice must preserve the specified types, direction, and semantics.

The concrete native boundary and the use of native types by Ama templates follow
[TPL-13](../language/templates.md#tpl-13). Generate the companion's declarations
from its resolved concrete runtime interface under that rule. A separate static
module under [TPL-12](../language/static-results.md#tpl-12) has no native entry
or runtime ABI; a concrete export selected as an ordinary native field type
contributes that field's normal resolved Type. Static graph inputs and outputs
add no C entry arguments, output storage or native execution protocol. Inline
static bindings and static output definitions are not native-interface forms.
The generated header and the companion keep the existing typed-field and
resource contracts, including declared concrete Ama ownership and references
under CABI-03/CABI-05.

The native interface has two separate outcome channels. Declared runtime Ama
results carry application values, including [ERROR-01](diagnostics.md#error-01)'s
condition-capable failure module with its contained description. The generated
technical-status output reports whether native execution itself succeeded under
PROTO-09. A true application failure value can therefore accompany technical
success. It does not become a nonzero technical status or select the call's
`:` handler. Failure to construct that result uses the technical channel and
the ordinary cleanup rules.

### Preserve nominal type identity

<a id="cabi-02"></a>

**CABI-02.** Type interpretation must follow declared module identity and its
registered representation, not an unchecked guess based on a parameter's name.
Native modules may use `void *` internally for opaque resources, but such a
pointer is not a general Ama reference, arithmetic value, or freely castable
language-level universal value.

### Pass typed values and bounded borrows

<a id="cabi-03"></a>

**CABI-03.** Native representation follows the declared module type; no universal
raw-word cast substitutes for type checking. The compiler applies owned-value
copying or reference-handle copying when binding Ama interface storage. A native
entry then receives the declared C value or a typed borrowed view of that already
bound storage; the view must not turn a value argument into an alias of its
caller's original owned object. Typed output views refer to target result storage.
The literal constructor separately receives immutable UTF-8 bytes and length.
A reference view is resolved from a checked handle for the permitted access and
cannot be retained after the native operation returns. Exact generated adapter
prototypes are engineering artifacts derived from these value/lifetime rules.

### Describe native resource representation

<a id="cabi-04"></a>

**CABI-04.** Native type contracts cover layout/alignment, construction, decoding,
cloning, destruction, optional conversions, and failure cleanup. Mutable C globals
must not substitute for instance state. The layout or address of a live native
payload is not implicitly moved during Ama invocation or pool growth. Internal
replacement storage may require a typed transfer implementation that preserves
native resource invariants; it is not permission to relocate live pool slots.
[RT-02](runtime.md#rt-02) may permit omission of unobservable instance machinery,
but an optimizer must not infer that an arbitrary native payload is movable or
that a hook has no effects. Any observed native address remains valid for its
declared lifetime, including bounded views; storage elision must preserve that
contract and all required lifecycle calls.

An authored payload layout or published typed view is binding for native uses
within its supported build. Its size, alignment, and any declared field access
must agree with generated storage and adapters. This is distinct from exposing
the enclosing pool slot's private identity, busy, cursor, or return-link records.
Native code must use generated access operations for those runtime services;
it must not recover private metadata by pointer arithmetic, guessed offsets,
or casts to an independently reconstructed slot struct. Changing private layout
requires no source-language change when its native observations remain valid.

### Keep Ama references out of opaque payloads

<a id="cabi-05"></a>

**CABI-05.** There are two explicit state domains:

| Domain | Persistent contents | Ownership and copy authority |
|---|---|---|
| Compiler-visible Ama fields | Owned instances/lists, reference variables/lists, interface values, execution/control metadata | Compiler/runtime |
| Opaque native payload | Native scalars, buffers, file/socket/device handles, other native-owned resources, and the restricted external library control resources below | Native lifecycle hooks |

All persistent ownership of instances in the component's Ama graph and all
persistent Ama references MUST be represented by declared Ama fields. An opaque
payload must not hide Ama handles, addresses of instances or fields,
list-buffer addresses, caller connections, or
saved execution positions. Encoding such identities as integers or bytes does
not exempt them from this rule. Raw `void *` is permitted only for native resources
under their lifecycle contract, not as an untracked Ama reference or owner.
FLOW-05's declared local reference cells and checked cursor identities are
compiler-visible control storage governed by CONT-02/CONT-04, within this Ama
domain; they must not be hidden in native payloads.

Native code may access compiler-visible fields and explicitly supplied typed
views during the current operation. Such views and handles are borrowed and may
not escape into opaque state, C globals, callbacks retained by external libraries,
or asynchronous work. To retain a relationship, store a checked reference in a
declared Ama reference field through the generated access contract. Native code
must not edit generation/busy/return metadata or bypass acquisition checks.
Native operations that mutate generated list storage must use the generated
access contract and preserve LIST-03/LIST-13's membership and incarnation updates.

The runtime clones and fixes up the visible ownership/reference graph. Native
`=` copies only native payload; it never receives a graph-walking or Ama-handle
fixup responsibility. There is no additional tracing protocol for hidden references.
This boundary applies equally to default values, parameters/results, list elements,
ordinary clones, and `%` copies. C remains capable of arbitrary operations, but
violating this native contract is unsupported native code, not defined Ama behavior.

#### A native wrapper may own an isolated library host resource

The sole additional control-resource case is an `@C` wrapper acting as the host
of a separately compiled Ama library under [EMBED-07](c-library.md#embed-07).
Its opaque payload may own that library's execution context and retain generated,
checked host-control handles for instances in that owned context. These handles
are native API capabilities: they are not ordinary Ama reference values, borrowed
views, cross-context aliases, or handles that the enclosing runtime can resolve.
The child context is a separately disposed native resource, not an invisible
owned edge in the enclosing Ama graph. CABI-05's prohibition still covers
ordinary Ama handles, addresses, current/ancestor context control handles, and
encoded equivalents. This exception does not permit borrowing another context's
instances or hiding references back into the parent.

Construction may create the child context, initialize its constants, and construct
explicit exported instances without running their bodies. Actual entry and resume
occur only in explicit wrapper operations. Destruction disposes of child resources
without executing suspended Ama statements. The wrapper must retain the library's
code and required native operation records until disposal and completion of any
callback that can still use them; unloading an image does not cancel those users.
The canonical lifecycle and shutdown contract is
[EMBED-06](c-library.md#embed-06).

The baseline owning wrapper omits `=` and is therefore statically noncopyable,
including through an owned enclosing field. Neither ordinary copying nor `%`
copying can duplicate a child context, its control handles, or native callback
registrations implicitly. Supporting an independently reconstructed child requires
an explicit conforming clone contract; sharing a mutable child context between
copied payloads does not implement PROTO-06. This exception does not add a seventh
`.abi` protocol or make the enclosing runtime trace the child's graph.

### Generate a usable native authoring interface

<a id="cabi-06"></a>

**CABI-06.** Provide an explicit native-scaffolding command that derives a native
interface header and an optional companion `.c` skeleton from the validated Ama
interface and native ABI contracts. The `.c` skeleton MUST directly include its
generated interface header. That header MUST be self-contained: including it as
the first project header in a C translation unit supplies all declarations and
definitions needed for the native entry signature and permitted typed access to
parameters, results, state and ABI services. It explicitly includes required
standard, native representation and generated dependency headers. Complete type
definitions must be available wherever C value passing or permitted field access
requires them; forward declarations suffice only where the declared opaque/view
contract permits them. This does not expose resource internals beyond the native
module's declared C representation/view contract.

The author MUST NOT have to copy generated declarations, discover missing type
headers, include the final project assembly, or rely on a previous include or
forced include to obtain those types. Arrange forward declarations, type
definitions and guarded dependency headers so legal module-reference cycles do
not create incomplete-type uses or recursive inclusion failures. Declaration
headers must not pull in Ama companion `.c` files, native entry/hook bodies or
project runtime/storage definitions; their reuse in the project assembly must
not duplicate types, objects or bodies. Inline utilities supplied by required
standard/external C headers remain ordinary dependencies.

Generated authoring contexts MUST establish the required NAME-05 prefix for the
header and the skeleton body under both independent C checking and final assembly,
with identical module-qualified identities. A header cleans up its temporary
naming context after inclusion; the skeleton establishes the context for its body
explicitly instead of relying on macros leaked by that header. Each contribution
performs NAME-05's prescribed cleanup before another module's context begins.
Native representation headers remain authored C inputs checked by the C compiler;
this requirement does not require the Ama compiler to parse C or infer missing
declarations.

Generation MUST also provide machine-readable C-tooling configuration for the
scaffold, including its working directory, include search paths, required defines
and language/toolchain flags; a compilation database is an acceptable format.
With those settings, a C language service can analyze the scaffold without the
execution backend or a prebuilt final project assembly. No guarantee is made for
editors without the relevant C-analysis support. Normal generation refreshes
owned headers/configuration; creating or replacing handwritten companions follows
COMP-04's explicit-request and output-ownership rules.

#### State the target and compatibility assumptions

The supported backend must document its native ABI contract edition and the
target/build assumptions needed to interpret the generated declarations. The
generated authoring artifacts and C-tooling configuration together must identify:

- The target architecture/platform and C language/toolchain mode, including
  required extensions and settings used to verify native-entry inlining.
- The target C data model and calling convention relevant to exposed types:
  pointer and `size_t` widths, required alignments, aggregate layout, and any
  ABI-affecting packing or compilation options. Use target declarations and
  checked properties, not an assumed universal layout for `bool` or `AmaStatus`.
- The actual generated/public native declarations, qualified identity scheme,
  representation headers, and external C dependencies needed for that build.
- Which compiler/native-ABI editions and toolchain configurations are supported
  together, and the changes that require regeneration, rechecking, or rebuilding
  affected native artifacts.

The format of this information and its generated identifiers are engineering
choices; no new `.abi` record or runtime capability registry is introduced.
The selected configuration must be explicit and reproducible. Cross-compilation
uses target properties rather than the compiler host's pointer/size layout.
Identical C type names or successful checks under a different configuration do
not establish compatibility. Do not combine stale generated headers, incompatible
representation definitions, or prebuilt external dependencies with a new assembly
by assuming that their layouts happen to match. The build must validate recorded
configuration and checkable declarations/layout properties against its supported
contract, and diagnose unsupported declared configurations or detected mismatches
before accepting a verified executable. Native authors and integrators supply
accurate external-library headers and ABI/build requirements; the compiler does
not recover or prove an arbitrary opaque library's ABI by inspecting its binary.

Changing a private generator/runtime implementation need not change the native
contract edition when all published obligations still hold. A change to exposed
types, signatures, naming, status meaning, ownership, or lifetime is a native
contract change even if C compilation would still succeed. No backward- or
cross-platform binary compatibility promise follows from this specification;
supported combinations require explicit evidence. Regenerate and rebuild affected
artifacts when compatibility has not been established, rather than preserving
an obsolete interface as an alternative contract.

Acceptance requires an isolated C syntax check with the generated interface header
as the first project include, a syntax check of the untouched generated `.c`
skeleton using the supplied tooling settings, and a composed project check proving
agreement of types, names and include ordering. Exercise transitive types, equal
native helper basenames in different modules, supported reference cycles, repeat
inclusion and temporary naming-context cleanup. These are declaration/scaffold
checks, not permission to link individual native modules separately or relax
native-entry inlining. Complete component-library packaging and its separate
public-header checks follow [EMBED-01](c-library.md#embed-01) and
[EMBED-06](c-library.md#embed-06).

The acceptance evidence must identify the target/configuration actually checked.
Exercise recorded ABI-affecting configuration or representation mismatches and
reject unsupported declared combinations. Verify size/alignment/type agreement with the target
C compiler where applicable; a text comparison of prototype spellings alone is
insufficient. Lifecycle, bounded-borrow, status, and execution probes are still
required because compatible C declarations cannot prove those behavioral rules.

## Symbolic protocol registration in `.abi`

### Declare the six fixed native protocols

<a id="proto-01"></a>

**PROTO-01.** Native type contracts use six fixed symbolic bindings. Every native
type must provide `+` and `-`; the other four capabilities are optional. Its
companion `.abi` file explicitly binds a symbol to a native C function. A binding both declares support and supplies its
implementation; no separate support flag or runtime registration is needed.

| Binding symbol | Meaning | Successful result |
|---|---|---|
| `+` | Create initial native state | Initialized destination state |
| `-` | Destroy native state and release resources | Finalized state, with any cleanup error reported |
| `\|` | Initialize native state from literal bytes | Initialized destination state |
| `?` | Convert current state to a condition | `bool` |
| `[]` | Convert current state to a list position | `size_t` |
| `=` | Independently clone native owned data | Constructed destination state |

For example, `stdlib/modules/integer.abi` may contain:

```text
+ initialize
- destroy
| from_literal
= clone
? to_condition
[] to_index
```

Function names are chosen by the library author and must be valid identifiers
under the supported C toolchain. The fixed protocol symbols are language-neutral;
English words such as `condition` or `index` are not reserved Ama names. The
`.abi` extension is the selected technical file suffix. Each function
identifier is local to its declaring native module and names
`AMA_NATIVE(identifier)` under NAME-05. Equal helper basenames in unrelated
companions cannot cross-bind. Native definitions use that naming facility;
wrapping conflicting unqualified definitions in adapters is not isolation.

### Read explicit bindings, not C naming conventions

<a id="proto-02"></a>

**PROTO-02.** The compiler reads these declarations directly, without parsing C
or discovering registrations through macros or name conventions. It validates
supported symbols, duplicate bindings, the associated native type, and uses of
its capabilities. Missing `+` or `-` is a compile-time error, including a missing
`.abi` file for a native type. Using an optional operation without its binding
is a compile-time error, not a runtime capability search. Unknown or duplicate protocol bindings are errors; the file is not an
open-ended syntax/plugin registry. The complete companion-file layout is
specified in PROTO-11.

C declarations remain in the native/generated header contract and definitions
in the companion `.c`. The generator emits typed uses so the C compiler verifies
that each bound function has the required state, output, and status signature.
An `.abi` file contains no C bodies and must not duplicate the native state layout.

### Use checked native transport signatures

<a id="proto-03"></a>

**PROTO-03.** The following signature patterns specify read-only payload input,
typed output, and the common status result. Hook and payload names shown here
are illustrative under the reading convention above:

```c
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(State) *state, bool *result);
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(State) *state, size_t *result);
```

`State` is an illustrative native-author local name for the payload representation;
`AMA_NATIVE` qualifies it and the hook names with the module prefix under NAME-05.
`AmaStatus` is the common code/message/length status defined in ERROR-03 through
ERROR-05. These are conversion helper signatures, not the void-return entry
interface of a native Ama module. On success the helper initializes its
output. On failure the output is not consumed.

#### Construct an integer from a host size without decimal text

The fixed standard-library `..integer` module additionally supplies this typed
native bridge in its representation header and companion implementation:

```c
AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination, size_t value);
```

The name `from_size` is fixed for this bridge, qualified by that module's
NAME-05 prefix; `State` here illustrates its authored payload type as above.
Generated declarations bind the actual payload type explicitly. Generated list
code invokes this helper only for the exact standard-library identity
`..integer` when constructing LIST-04's `length` result. Missing or incompatible
bridge declarations are native-interface build errors. A project module with a
similar name does not acquire this contract.

`destination` is fresh, suitably aligned native payload storage, not a live
integer to overwrite. Its generated owned fields have already been constructed
under LIFE-01. The helper constructs exactly the mathematical nonnegative integer
`value`, for every input from zero through `SIZE_MAX`, and makes the payload live
only on success. It must not report invalid-index or range refusal for a valid
`size_t` input. Allocation or other technical failure uses the same `AmaStatus`
contract, releases partial native resources, and leaves the payload non-live;
generated cleanup finalizes previously constructed Ama fields. A successful
payload uses the ordinary `-` lifecycle and must round-trip through the type's
`[]` conversion to the original `size_t` value.

This is an explicitly required helper of one fixed library module, not a seventh
`.abi` binding, an extensible conversion registry, a universal numeric cast, or
permission to inspect arbitrary native layouts. The `.abi` table still contains
only the six symbols in PROTO-01/PROTO-11. The generator knows this helper's typed
declaration and module identity; it does not parse its C implementation or infer
it from arbitrary C function names. Library algorithms can use the bridge under
the same fresh-storage contract; other Ama types do not acquire it implicitly.

**Purpose.** List length already exists as structural `size_t` metadata. Requiring
decimal formatting and then literal parsing adds temporary bytes and two
conversions merely to return the same number. The bridge constructs the usual
owned Ama result directly. Source literals and omitted source defaults still
use their selected `|` or `+` constructor; this bridge does not introduce a new
Ama literal syntax or change ordinary literal decoding.

**Implementation explanation.** The integer companion can reserve the required
native limbs and populate them directly. For example, for a representable limb
base `B >= 2`, repeatedly emit `value % B` and replace `value` by `value / B`,
with a single zero limb for input zero. All division is unsigned and bounded by
the original `size_t`; no decimal bytes, signed narrowing, or out-of-range shift
is required. Other integer representations may use an equivalent exact
algorithm. Representation choice belongs to `..integer`, rather than generated
list code. Tests must cover zero, one, limb boundaries, `SIZE_MAX`, round-trips,
and injected preparation failure with no live partial payload. LIST-04 selects
this bridge as its single native payload constructor under LIFE-02, never
default construction followed by overwriting the initialized integer.

### Conversions inspect current state

<a id="proto-04"></a>

**PROTO-04.** Conversions inspect current state without modifying it, executing
the Ama module body, resetting its interface, or reading a potentially stale
cached `@>` result. They must not cause observable side effects or retain borrowed
pointers. Runtime access follows the acquisition/lifetime rules; an internal read
of the current executing instance must not attempt external busy-instance reentry.
Conditions request `?` when evaluated, including before every iteration of a
nonempty loop. An empty loop requests it once on entry under FLOW-02.
Indexed access requests `[]` when selecting a position. Ordinary result reads
through `>>` retain their existing nonexecuting stored-result semantics.

### Reject invalid indices distinctly

<a id="proto-05"></a>

**PROTO-05.** Index conversion must reject values that cannot represent a valid
nonnegative host position, including negatives and values above `SIZE_MAX`.
The list runtime separately checks the converted position against list length
using the zero-based bounds in LIST-02, without an index-origin adjustment. An invalid indexed invocation/read is an
acquisition refusal eligible for its attached `:`; standalone invalid access
uses its operation's failure protocol. A technical helper failure is not silently
converted into false or a valid position. The explicit status ABI must distinguish
invalid-index refusal from technical failure. `bool` and `size_t` are native
transport types only, not built-in Ama types. No integer type automatically gains
truth semantics; a library must explicitly bind `?` if it wants that behavior.

### Clone native data independently

<a id="proto-06"></a>

**PROTO-06.** The `=` binding explicitly supplies independent native data cloning:

```text
= clone
```

Without this binding, a native type does not support owned copying. There is no
separate prohibition flag, implicit `memcpy` fallback, or inferred clone support
from a similarly named C function. An absent `.abi` file also supplies no clone
capability, and also fails the mandatory lifecycle-binding checks. For example,
a file-resource type may omit `=` while an integer type
binds `= clone`. The binding is metadata, not a new assignment operator
in Ama source and not an equality-comparison protocol.

The helper constructs an independent destination native payload from a read-only
source and reports explicit success/failure. Independent payloads may retain
counted shares of explicitly immutable native backing storage under the rules
below; their mutable state, resource disposition, and observable value behavior
remain independent. The helper must obey the fresh-storage and
failure-cleanup protocol in LIFE-01 through LIFE-05. It does not contain, clone,
or redirect persistent Ama handles; COPY-05 handles generated Ama fields only.
The runtime handles saved execution positions separately; ordinary and `%` copying
invoke the same data-clone contract, not different native clone modes.

#### Share immutable native backing without sharing mutable values

The native type contract may define a shared backing record containing immutable
bytes, their immutable size/description, and lifetime bookkeeping. Each native
payload owns its own share and any mutable per-instance state. No conforming
operation, borrowed view, external native owner, or callback may modify the
shared bytes while any payload or borrow can observe them. Shared files, sockets,
device state, mutable buffers, or persistent Ama identities cannot be treated as
immutable backing by declaration alone. CABI-05 continues to prohibit hidden Ama
handles and pointers in both the payload and its backing records.

The backing address and share count are not an Ama instance identity. Sharing
is permitted only when the declared native/library contract allows immutable
backing aliasing and preserves any promised address or resource-identity
observations. A bounded native view may expose its actual byte address during
the permitted operation; this does not turn that address into an Ama identity
or make lifetime bookkeeping mutable application state.

Construct each destination payload by invoking `=` normally, with fresh storage;
do not replace that invocation with an untyped payload `memcpy`. A checked share
count is a permitted lifetime algorithm: increment before publishing a new
payload, reject count overflow as technical clone failure without changing the
source or destination, decrement exactly once when that payload is finalized,
and free only after the final owner and its permitted borrows are finished.
Explicit bounds or another exact ownership algorithm are also permitted if they
prove the same invariant. Cloning failure must undo every share acquired by that
failed constructor; failure of a later child or reference-fixup step releases
the already successful prepared payload through ordinary rollback cleanup.

Borrowing does not transfer a share or extend CABI-03's lifetime: the backing
must stay alive throughout each operation's permitted view, and no view escapes
the operation. A value-changing operation must prepare a new immutable backing
and perform the ordinary replacement protocol. There is no permission to defer
a required allocation to an unspecified future mutation, mutate shared bytes,
or introduce general copy-on-write semantics.

The simplest implementation confines the record and its non-atomic share count
to one execution context. Sharing immutable native backing across contexts
requires an explicit native resource contract satisfying
[PAR-05](../language/async-and-parallelism.md#par-05): synchronized lifetime
bookkeeping, safe concurrent read access, and no shared mutable Ama state or
cross-context Ama handles. It cannot be inferred from immutability of the byte
array alone or used to bypass serialized message boundaries.

Acceptance must exercise two copies with distinct Ama identities sharing one
immutable buffer, destruction in both orders, per-instance replacement without
altering the other value, rollback after acquiring a share, reduced-width share
count overflow, and borrows held during their permitted operation. A
cross-context implementation additionally needs concurrent ownership/cleanup
tests. These checks establish independent value semantics, not merely equal
addresses or lower allocation counts.

### Derive copy capability over owned types

<a id="proto-07"></a>

**PROTO-07.** For Ama-defined types, owned-copy capability is derived from all
owned fields and recursively owned element types. References and reference-list
entries do not require their target types to support cloning. An owning list
requires clone support for its element type; temporary emptiness does not imply
a different static capability. Known unsupported owned copying is a compile-time
error, including an unsupported nested native field. Pure reference copying stays
valid even when the target resource cannot be cloned. These static capability
checks apply to every owned-copy operation, including a receiver and source that
will resolve to the same runtime identity. For a statically valid ordinary copy,
COPY-06 makes that identity match a no-op without invoking native cloning.
Explicit `%` copying still requires distinct source and destination identities.
The compiler does not accept copying a noncopyable type by guessing that aliases
will coincide at runtime.

Derive capability over the finite resolved type graph. Add an edge `T → U` for
each owned scalar or owning-list field of `T` whose element type is `U`, including
parameters, results and persistent fields, but excluding shared constants and
all non-owning edges. Initially mark every executable type as potentially
copyable. Remove each native type lacking `=`, then repeatedly remove any type
with an owned edge to a removed type, until no change occurs. The remaining set
is the greatest fixed point of “native payload is copyable and every owned
dependency is copyable”. This terminates after at most one removal per type.
A cycle through owning lists is not itself a reason to reject cloning: finite
instance trees of a recursive type remain copyable when no owned dependency
reaches a noncopyable native type. Eager allocation cycles remain a separate
TYPE-04 check. There is no inference from current list length.

A supported clone may still fail at runtime, for example on allocation. Clean up
partially created native/owned data and clone-context metadata, preserve the
source and previous destination, then follow the technical-failure diagnostic
protocol. Never silently share a mutable owned external resource to make cloning
succeed. A registered clone must implement independent data semantics;
PROTO-06's explicitly immutable backing is permitted because neither value can
change the other's data or invalidate its lifetime. Shared access to a mutable
Ama object is expressed through reference storage instead.

### Select one fresh constructor

<a id="proto-08"></a>

**PROTO-08.** `+` initializes fresh native storage without executing the module
body. `|` constructs a value from immutable UTF-8 bytes and an explicit length;
it is required only when literal initialization is used. Default construction,
literal construction, cloning, and PROTO-03's fixed `..integer` size bridge are
distinct construction paths over fresh native storage, as specified in LIFE-01
through LIFE-05; a value is not initialized
twice. Construction failures release partial resources and leave no
published partially live value. The compiler-generated lifecycle combines native
hooks with owned-field construction/cleanup; hooks must not double-destroy fields
owned by that generated lifecycle.

`-` finalizes the native state without executing the Ama body or yielding.
The runtime invokes it automatically on deletion, destination replacement,
owner/runtime cleanup, and finalized shared constant value storage. Types with no native resource still provide a valid no-op
destroy hook; all successfully constructed initial states must be destroyable.
Owned-field ordering and failed-constructor responsibilities are fixed in
LIFE-01 through LIFE-05.

### Report technical status explicitly

<a id="proto-09"></a>

**PROTO-09.** All native operations use one explicit technical-status/error
contract: default initialization, literal decoding, cloning, the fixed
`..integer` size bridge, condition/index conversion, destruction, and native
module execution. The contract distinguishes
success from failure and carries a reason code plus optional diagnostic context.
The runtime supplies Ama source location and execution-path context. The status
uses the code/message/length representation and immutable-message lifetime in
ERROR-03 through ERROR-05. Full code allocation and operation-specific parameter
placement remain ABI details; no extra `.abi` error symbol or separate
per-operation error mechanism is introduced.

Each bound helper reports status through that contract. A native module entry
retains its void-return/output-parameter interface and reports the same technical
status through a generated service output, separate from declared Ama results.
Application-level failures remain ordinary declared results. Expected invalid
index refusal must remain distinguishable from an unrelated technical conversion
failure. Native functions return normally before the runtime handles errors;
no exceptions or extra `:` behavior are introduced.

### Continue cleanup after destruction errors

<a id="proto-10"></a>

**PROTO-10.** An error reported by `-` does not cancel overall cleanup. Continue
releasing other owned resources, retaining the primary error and recording
subsequent cleanup errors as additional diagnostics. Do not overwrite the original
cause or recursively abort cleanup on each new failure. Diagnostic fallbacks must
remain usable without successful heap allocation.

The type contract must define the disposition of each native resource after a
failed destroy. Automatic destruction is attempted at most once for that state;
the runtime must not retry blindly or expose a partly destroyed value as live.
A failed external release may leave an external resource unreleased, which must
be reported rather than represented as successful cleanup. The slot ceases to
be live under the generation rules. Error reporting cannot promise recovery or
rollback of external side effects.

Construct and validate a replacement before destroying the old value, so clone
or construction failure preserves the old destination. Once old-value destruction
begins, a destructor error is a terminal technical failure, not a transactional
rollback: never resume use of the partly destroyed destination. Keep enough
construction-state bookkeeping to finalize remaining old and newly constructed
resources exactly once on this failure path.

### Read the native binding table exactly

<a id="proto-11"></a>

**PROTO-11.** The `.abi` file is a UTF-8, line-oriented binding table. Each
record starts at column one and contains exactly one supported symbol, exactly
one ASCII space, and one module-local C-function identifier under NAME-05.
The supported symbols are
`+`, `-`, `|`, `=`, `?`, and the contiguous token `[]`. Function identifiers must
be valid under the supported C toolchain; the compiler does not infer capabilities
from their spelling. Function signatures and expressions are not written here.

Records may appear in any order. Empty blank lines, comment-only lines, and trailing
comments are allowed. Trailing spaces without a following `#` and space-only lines
are invalid. `#` begins a comment through the physical line
ending; it cannot occur within a function identifier. LF and CRLF are supported,
and the final newline is optional. Tabs, a lone CR, indented records, and extra
non-comment tokens after the identifier are invalid. Arbitrary leading spaces
belong to comment-only lines; spaces immediately before `#` belong to the comment
suffix under SYN-02. They carry no structure in this flat companion format;
Ama's standalone-comment block rule does not apply here. Ama multiword-name rules do not
apply to the C identifier; it must remain one valid C toolchain identifier.

Each symbol occurs at most once. Unknown symbols, duplicate symbols, malformed
records, and missing mandatory `+` or `-` are compile-time errors with companion
file/line/column diagnostics. Records do not declare sections, nesting, includes,
macros, conditional registration, or executable code. This fixed table is a native
interface description, not a second programming language or an extensible parser.

Example:

```text
# Native integer operations; record order has no meaning.
+ initialize
- destroy

| from_literal
= clone
? to_condition  # Optional condition support.
[] to_index
```

## Complete lifecycle and cleanup protocol

### Track fresh, constructing, live, and finalized storage

<a id="life-01"></a>

**LIFE-01.** Native storage is fresh, constructing, live, or finalized; compiler
bookkeeping records which owned fields and native payloads are live. Fresh means
suitably aligned storage whose resource representation has not been constructed;
zero-filled bytes alone do not imply a live value. Construct owned Ama fields
in declaration order (parameters, results, then persistent body declarations)
without executing module bodies. Construct the native payload after the fields
on which it may depend are initialized. Native hooks do not own these fields.
Construct each field by its selected initializer or source-clone path. In a
clone, construct children from source data rather than running their declaration
defaults first. Declaration-order construction does not mean default construction
followed by overwriting resources.

New-instance construction uses the reserved final identity and instance context
from RT-12 before evaluating any field initializer. A standalone `&` may form
that identity during construction without reading or exposing partially
constructed contents. Interface reinitialization retains the existing callee
identity under CALL-08; clone preparation uses COPY-05's final destination
mapping instead of reevaluating self-reference defaults.

### Construct fields in declaration order

<a id="life-02"></a>

**LIFE-02.** Select exactly one payload constructor: `+` for a default, `|` for a
literal, `=` for a clone, or PROTO-03's fixed `..integer` size bridge when
LIST-04 requests it. Do not invoke `+` and then overwrite its resources
with another constructor. Constructors receive fresh destination payload storage.
A successful constructor makes it live; a failed constructor must release every
resource it acquired, report status, and leave that payload non-live. The runtime
does not call `-` on that failed payload. It does finalize previously constructed
children in reverse construction order. Report any secondary cleanup errors
without replacing the original construction error.

### Destroy each resource once

<a id="life-03"></a>

**LIFE-03.** Destruction invalidates external access before releasing resources,
then calls native `-` while owned child fields still exist, and destroys the
owned fields in reverse construction order. After `-` returns, successful or not,
the native payload is never used or destroyed again. Continue through all owned
children even if one cleanup fails. Do not execute child bodies, follow non-owning
references for destruction, or access caller/owner state through escaped pointers.
The slot then becomes free or retired under the generation protocol.

### Prepare replacements before destructive commit

<a id="life-04"></a>

**LIFE-04.** Copy preparation validates identities, ownership overlap, source-tree
availability and clone capabilities, validates that replacement will not destroy
busy/acquired objects in the old destination ownership tree, allocates unexposed
destination storage,
constructs the independent ownership tree, and fixes visible internal references.
The mapping of the source root uses the final destination identity, not a temporary
staging identity. No new copy is externally visible during preparation. Commit
starts only after preparation succeeds: finalize old destination contents and
publish the replacement under that destination's stable identity. If commit cleanup
fails, stop Ama execution and finalize remaining prepared/old resources exactly
once; rollback is neither attempted nor promised. Generated storage/transfer
helpers implement this protocol without introducing new source operations.

### Dispose a failed execution consistently

<a id="life-05"></a>

**LIFE-05.** Technical failure first reports the active diagnostic context, then
ends the active execution context and releases its outstanding acquisitions/return
links through explicit runtime bookkeeping, not C exception unwinding. Failed
execution is not resumable. Runtime disposal destroys each remaining root
ownership tree in that context exactly once. Independent roots, including
explicitly host-owned roots, are destroyed in reverse order of their successful
creation. A creation counts only when construction succeeds and its root
identity is published;
failed creation adds no root to this order. Explicit earlier disposal removes
that root from the remaining order. Replacing a root's contents while preserving
its identity does not change its position. Nominal type, pool layout and slot
reuse do not determine the order. Each root's owned tree follows
[LIFE-03](#life-03).

After all roots, disposal destroys shared constant storage in reverse
initialization order. Cleanup continues after errors and preserves the primary
failure under [PROTO-10](#proto-10). At a normal completed/yielded host boundary,
disposal uses the same ownership cleanup rules. A host may not destroy a running
or waiting instance; there is no preemptive cancellation of active Ama execution.
The explicit task
cancellation protocol does not change this boundary. Outstanding native operation
records follow ASYNC-09: detachment must prevent later callbacks from accessing
destroyed Ama state, while any required native record remains alive until its
external completion is finished. Native cleanup hooks cannot enter, resume or
restart Ama bodies, reenter the current or an active ancestor context, or create
new Ama work.

Cleanup does not implicitly dispose independent peer contexts. An owning native
wrapper must, however, dispose its exclusively owned foreign child resources
through their supported cleanup interface under
[EMBED-07](c-library.md#embed-07). This is resource destruction, not Ama execution
or callback reentry. Preserve the acyclic context-ownership forest and release
each successfully acquired child resource exactly once, including a yielded or
failed child. Child disposal follows its own root/constant cleanup and the
deferred-native-activity and code-lifetime rules in
[ASYNC-09](../language/async-and-parallelism.md#async-09) and
[EMBED-06](c-library.md#embed-06); it does not resume the child's saved
continuation. Further exclusively owned descendants follow the same rule.
Ownership cleanup does not grant
authority to dispose an unrelated context.

**Informative example.** If a host successfully creates independent roots `a`,
`b`, then `c`, context disposal destroys `c`, `b`, then `a`, followed by the
constants. If the host has already disposed `b`, only `c` and `a` remain, in that
order. Replacing `a`'s contents does not make it newer than `c`. This resource-stack
order makes cleanup predictable independently of storage allocation choices.

**Informative example: owned children and independent peers.** Context `A` owns
a native wrapper `W`; that wrapper exclusively owns child context `B`. Context
`C` is independently owned:

```text
A -> owned W -> exclusively owned B
C (independent peer)
```

Disposing `A`, including after its technical failure, finalizes `W` and disposes
`B` exactly once. If `B` has yielded, disposal destroys its saved state without
executing the statements after `%`. Context `C` remains live and can continue
under its own host policy. Native callback records required after `B` is detached
retain their native storage and code lifetime, never access to destroyed Ama
state.

Generated C parameter names, private type aliases, and helper decomposition are
implementation work wherever no rule fixes them. This freedom does not rename
`AmaStatus`, change its required declaration, replace NAME-05's naming facility,
or alter the fixed `..integer` `from_size` bridge. Concrete declarations must
follow CABI-06 and preserve the required types, preconditions, order, lifetime,
and status guarantees; their engineering spelling is not an unresolved language
semantic and cannot weaken the native boundary.
