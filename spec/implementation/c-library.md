# Compiled Ama libraries and C hosts

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: a component is an execution boundary](#explanation-a-component-is-an-execution-boundary)
- [Choose and build a library interface](#choose-and-build-a-library-interface)
- [Create contexts and explicit exported instances](#create-contexts-and-explicit-exported-instances)
- [Call, resume, and read through the host interface](#call-resume-and-read-through-the-host-interface)
- [Transport values without exporting live storage](#transport-values-without-exporting-live-storage)
- [Keep failure boundaries and diagnostics explicit](#keep-failure-boundaries-and-diagnostics-explicit)
- [Link, identify, and unload components safely](#link-identify-and-unload-components-safely)
- [Own a foreign component inside a native module](#own-a-foreign-component-inside-a-native-module)
- [Return from nested calls before yielding](#return-from-nested-calls-before-yielding)

<!-- /chapter-contents -->

## Explanation: a component is an execution boundary

An Ama component can be compiled into a library used by a larger C application.
The application owns explicit library contexts and instances, supplies inputs,
receives independently owned results, and decides what to do when execution
finishes, yields, or fails. A library has no hidden process entry or scheduler.

The same interface can be used by the C companion of an Ama native module.
That companion acts as a C host for a **different execution context**. It does
not call back into the currently executing Ama dispatcher or turn an ordinary
Ama module call into a C-stack activation. Inside each component, generated
Ama control transfers continue to follow [STACK-01](compiler.md#stack-01).

**Example architecture.** A C document editor links an Ama validation component.
A different Ama application may also use that component through a native
`validator` adapter. The editor and adapter both use the component's published
C header. Neither needs its pool layouts, generated labels, or handwritten
declarations reconstructed from its compiled binary.

This chapter defines the library boundary. The [native interface](native-interface.md)
defines authored native companions inside a component; its authoring header is
not a substitute for this public library header. Exact API function/type names
in the examples below are illustrative. Supported generators must publish
their actual checked declarations and reproduce the required semantics.

## Choose and build a library interface

<a id="embed-01"></a>

**EMBED-01.** Provide an explicit library-build configuration that selects the
exported executable module types, their public entry/resume/read operations,
and the input/result transport schema for each operation. Resolve the complete
Ama dependency closure and required native/external C dependencies. Diagnose
an unavailable operation, unresolved dependency, or unsupported transport at
build time; do not infer an export from filenames, public-looking C symbols,
or a type's basename.

Select an exported root by the full logical module path of an ordinary,
non-template concrete executable module. The root may have either an Ama `@!`
body or a concrete native `@C` implementation under
[TPL-13](../language/templates.md#tpl-13). An already concrete native root needs
no additional facade. To expose a generic Ama algorithm, define an ordinary
Ama facade with concrete runtime inputs/results and explicitly closed internal
specializations, then select that facade's module path.

This export contract does not select a template root through a static binding
map, a static-module export selector, or a generated specialization name. Reject
a static module or template definition selected as an exported root even if one
of its concrete specializations was discovered elsewhere in the component. Open
template distribution follows TPL-13's Ama-source contract; an unresolved constructor is
not a runtime C export.

Every runtime interface type and required dependency must be concrete before
generating the public header and transport schema. Each required Type/Template
binding must be determined under TPL-06; a Template-valued binding may retain
its own declared residual formals. Runtime C arguments do not choose an Ama
specialization or infer a missing static argument. Separate `@...` graph
results under [TPL-12](../language/static-results.md#tpl-12) contribute no runtime
C result fields or operations for returning types. A facade's named static
export selections must resolve independently of the host call context.

Checked descriptors and schema comparisons preserve each Type's complete nominal
identity. Equal runtime layouts do not permit substitution between different
nominal Types.

The configuration is a compiler/build input, not new Ama syntax or another
`.abi` protocol. Export selection does not change the module's nominal identity
or expose private fields. An explicit facade may provide a narrower interface
than the underlying module. A resume-only export cannot manufacture a saved
position: runtime resume still requires one.

For each supported target, the toolchain must support a static library artifact
and, where the target/toolchain supports shared libraries, a shared artifact.
Produce a self-contained public C header and machine-readable build/ABI
manifest alongside the library. The manifest records the component identity,
export/schema identities, ABI edition, target, calling convention, required
link inputs and ABI-affecting build settings. It is distinct from the generated
native-authoring declarations under [CABI-06](native-interface.md#cabi-06).

The component is generated as one project C assembly under
[GEN-01](compiler.md#gen-01). The surrounding application may have many C
translation units and may link several components. Component-internal helpers
and storage remain private; the public host API is the deliberate external
boundary. Public declarations use component-qualified types and symbols; the
header must not leak private runtime/native-authoring headers or an unqualified
internal `AmaStatus` declaration. If technical-status information is public,
express it through the component's explicit public transport contract. Linking
or loading a component must not execute Ama bodies, create
implicit instances, initialize Ama constants, install a scheduler, or define
`main`. Required C/platform runtime initialization is not Ama execution.

**Explanation and implementation.** Generate the library and header from one
validated export model rather than exporting arbitrary internal symbols and
then describing their accidental signatures. Compile the one assembly as a
library object, select target visibility/export settings, and retain only the
declared public entry points. Compile a separate C consumer with the public
header as its first project include; then link and run that consumer against
both artifact kinds supported by the target. Linking without making an API
call must have no Ama lifecycle effects.

### Example: export a concrete facade over an Ama template

**Example: complete `/example/demo/v1.0.0/src/forward`.** Assume the supplied Type supports ordinary
default construction and owned copying:

```text
@<
value
.inner.value type

@>
result
.inner.value type

@!
result
 value
```

**Example: complete `/example/demo/v1.0.0/src/integer worker`.** A separate static module fixes the
worker Type before the ordinary facade uses it:

```text
@...
.worker type
/example/demo/v1.0.0/src/forward
 .inner.value type < //integer
```

**Example: complete `/example/demo/v1.0.0/src/integer facade`.** The fixed library type `//integer`
provides the construction and copy operations used here:

```text
@<
value
//integer

@>
result
//integer

@!
worker
/example/demo/v1.0.0/src/integer worker
 .worker type

worker !
 value < value
 result > result
```

Select `/example/demo/v1.0.0/src/integer facade` and its supported entry/read operations in the explicit
library-build configuration, with a supported integer transport schema under
EMBED-04. The facade has no free static parameters; its worker is the checked
integer specialization of `/example/demo/v1.0.0/src/forward`. A second source facade with concrete
text inputs/results can independently use the text specialization. The C
consumer uses the generated declarations for each selected facade and supplies
values in its declared transport representation. No runtime type selection,
universal payload cast or knowledge of the worker's layout is needed.

`worker` is explicit permanent Ama state, independently owned by each facade
instance under EMBED-02. Internal calls retain the ordinary Ama dispatch and
lifecycle contracts; the facade adds no hidden default instance or shared
working state. Its private worker is not exposed as a pool address or native
authoring type in the public header. These source examples illustrate the
required contract and make no claim of existing template implementation.

## Create contexts and explicit exported instances

<a id="embed-02"></a>

**EMBED-02.** The public interface must provide explicit context creation,
exported-instance creation, instance disposal, and context disposal. Context
creation initializes that context's shared constants and runtime services,
but creates no implicit executable root and executes no Ama module body.
Each exported instance is an explicit host-owned root of its selected nominal
type, with recursively initialized owned state and default interface values.
Creation executes required native lifecycle hooks and declaration initializers,
not the instance's body. Failure cleans up every successfully constructed part
before returning failure; an incompletely created handle is never published.

A context factory that fails during allocation or shared-constant initialization
returns its technical failure diagnostic directly in an owned or documented
immutable public API result. It cannot require a subsequent diagnostic query
through a context handle that was never successfully published. Apply EMBED-05
and EMBED-06's diagnostic ownership/code-lifetime obligations to that result.
Failure to construct an instance after admission is a technical failure of its
already live context, which becomes failed and permits only diagnostics/cleanup;
the factory publishes no instance handle. A rejected creation request, such as
an unsupported exported type or busy context, constructs no default root and
does not fail an otherwise healthy context.

Entry on a separately created instance follows [CALL-08](../language/calls-and-results.md#call-08):
reset its interface even on the first entry, bind the supplied inputs, and
execute from entry while preserving its persistent body state. Its earlier
creation and this entry are two explicit operations. This differs from
[RT-11](runtime.md#rt-11)'s combined standalone startup, whose just-constructed
interface is used directly without a second reset. A backend must not duplicate
initialization within that combined operation or omit a reset from a later
explicit entry.

An available instance can retain results and, after yield, a saved position
between host calls. Resume preserves the interface and accepts no fresh inputs;
a fresh entry discards the saved position under the ordinary language rules.
Several exported roots may live in one context, but none is an implicit
singleton. They share that context's constants and type pools, not each other's
instance state. Different contexts have distinct pools, identities, and mutable
constant-lifecycle metadata under [PAR-01](../language/async-and-parallelism.md#par-01).

Instance disposal invalidates its host identity and destroys its owned tree;
disposing a yielded instance does not run its continuation. Context disposal
invalidates all remaining instance identities and performs the supported full
cleanup in the root and constant order defined by
[LIFE-05](native-interface.md#life-05). Operations on running contexts or instances
are rejected under EMBED-03; disposal is not asynchronous interruption. Contexts
that failed technically permit diagnostic extraction and cleanup, not further
entry, resume, or ordinary retained reads. Cleanup errors are reported while
remaining cleanup continues under [LIFE-05](native-interface.md#life-05).

**Invariant and tests.** Count native constructions, resets, and destructions
for separate creation/first entry, repeated entry, combined standalone startup,
yield/disposal, and partial construction failure. Exercise two roots in one
context and the same exported type in two contexts. Creating a context alone
must not execute an exported body or construct an unrequested executable root.

## Call, resume, and read through the host interface

<a id="embed-03"></a>

**EMBED-03.** Every context has one executor. Guard all host operations that
access its state, including entry, resume, retained reads, creation, disposal,
and result snapshot construction. A concurrent or reentrant attempt must be
detected and rejected before accessing mutable context state; different roots
do not circumvent the context guard. No API callback may reenter an active
context. Independent contexts may execute concurrently subject to
[PAR-02](../language/async-and-parallelism.md#par-02)'s native dependency constraints.

Separate API admission from the outcome of an accepted execution:

| Observation | Meaning | Effect on the target's Ama state |
|---|---|---|
| Rejected API operation | Invalid handle, unsupported mode, wrong schema/tag/argument count, invalid bounded-buffer shape, busy context/target, missing saved position, or another documented admission precondition | No input binding, reset, result publication, or continuation change |
| Accepted execution: completed | Entry/resume reached its normal root return | Completed state and retained results are available |
| Accepted execution: yielded | Entry/resume reached root `%` | Retained results and a saved position are available |
| Accepted execution: failed | A technical execution/transport construction failure occurred | Context is failed; only diagnostics and cleanup remain |

The rejection category is not a fourth Ama execution outcome under
[RT-14](runtime.md#rt-14), an application failure value, or the caller's Ama
last-refusal record. It belongs to this host API. Invalid mode/handle/schema
uses the documented rejection path, not an attempted call into arbitrary
storage. Caller misuse of the C language itself, such as passing a dangling C
pointer or an undersized buffer contrary to its declaration, is outside the
supported API contract; a checked handle is not permission to dereference it.

Validate admission before resetting or binding the target. Admission checks the
handle/mode, declared transport schema and discriminant, argument count, output
destination state, and explicit buffer shape: for example, null/length rules,
representable lengths and declared slices fitting within the supplied bounds.
These checks do not execute a type's semantic decoder or constructor. After
admission, a typed decoder rejecting input contents, failure to construct owned
input values, execute native hooks, or create required result snapshots is a
technical failure under [CALL-07](../language/calls-and-results.md#call-07), not a
rejection or rollback promise. Malformed typed contents cannot be classified as
an admission refusal merely to recover from a native decoder failure.

Build snapshots in unpublished temporary storage. Only a successful
completed/yielded operation or successful retained read publishes them into the
caller's initialized-empty output destination; failed preparation releases all
partial snapshots and leaves that destination empty. A retained read does not
execute a body, reset an interface, or consume a saved position. Unsupported
resume/read export selection is rejected, independently of the target's state.

**Explanation and implementation.** Acquire an exact non-reentrant context gate,
validate opaque handles, mode, schema and target availability, then run the
accepted operation. Do not use unchecked cached payload pointers to validate
it. Establish outcome/result ownership before releasing the gate. Exercise
simultaneous callers, reentry from a native helper, wrong-context and stale
handles, resume before entry or after completion, and invalid inputs. Rejected
operations must preserve the previous input/results and saved position.

## Transport values without exporting live storage

<a id="embed-04"></a>

**EMBED-04.** Every exported parameter/result has an explicit checked transport
schema. The public interface accepts bounded read-only caller input borrows
for the duration of an operation and constructs independent, correctly typed
Ama values inside the context. It must not retain a caller buffer after return
unless a separate explicit native-resource contract transfers its ownership.
Preserve nominal types, encoding, length/range constraints, and owned/reference
semantics; do not recover them from C field names or universal raw-word casts.

Output values are independently owned typed native snapshots or explicitly
serialized byte sequences, with the library's matching release operation.
They remain usable after the operation returns and, unless their declared
resource contract explicitly requires a live context, after instance/context
disposal. A result does not expose live Ama payloads, pool/field pointers,
checked Ama references, list buffers, or continuation records. Serialization is
a declared conversion, not a cast or raw-byte copy of an in-memory C struct.

The header documents initialization, ownership transfer, and release for every
owned transport type. An output destination must be initialized empty before
use; a nonempty destination is rejected before operation admission. Releasing
an initialized-empty value is a no-op, and releasing a live value leaves it
empty. Ordinary C struct assignment is not an independent clone and must not
create two releasing owners. When independent duplication is supported, provide
a documented explicit clone operation; otherwise obtain another snapshot by
another successful export/read. Any explicit move transfers sole ownership and
leaves its source empty. A caller must not release an uninitialized value.

Reject an export whose types have no supported explicit transport. Native
types require a documented encoder/decoder or public transport representation;
private payloads are not automatically serializable. Owning lists and structured
values require recursively specified schemas. A reference or graph export
requires an explicit value representation and reconstruction policy that does
not carry a checked Ama identity across contexts; references do not become
portable merely by encoding a slot number as bytes. Unsupported shapes fail
export validation rather than producing a partial schema at runtime.

The export/build metadata explicitly identifies each schema and its encoder,
decoder, and release/optional clone declarations under the selected public ABI.
Generated typed calls and target C checks establish agreement with those declared
adapters. This does not add a seventh `.abi` binding, infer a codec from a C
function's spelling, or require the Ama compiler to parse arbitrary C code.

An [ERROR-01](diagnostics.md#error-01) application failure remains **one** value.
Its C transport can be one owned record containing its discriminator, description,
and any declared typed details. False implies an empty description; true carries
the current description. These fields encode one Ama module, not an additional
Boolean Ama result or a technical-status channel. Copying/releasing the record
must preserve the description's ownership and lifetime.

Host object handles are an opaque control facility for explicitly exported
contexts/instances. They encode or validate the component, context, lifetime,
and generation needed to reject stale or foreign identities. They expose only
the operations in their export contract, not a portable Ama reference or a way
to retain the current component's hidden fields. Instance handles do not keep
a disposed context alive implicitly. Any explicit resource-retention contract
must be represented and released independently.

Validate such handles against live library-managed identity records before
dereferencing context/instance storage. An opaque handle can be a checked token
whose public representation hides its meaning; it must not be a freed C address
blindly dereferenced to discover whether it is stale. Retain or retire identity
history with nonwrapping generations so context disposal/recreation cannot make
an old handle valid again. This does not turn arbitrary invalid caller pointers
used for input/output C buffers into safe tokens.

**Invariant and tests.** Obtain two snapshots through repeated retained reads or
the explicit clone API, dispose their source instance, and verify each snapshot
until its own release. Exercise empty initialization/release, release leaving
empty, independent clone ownership, nonempty output rejection, and partial
marshalling cleanup; shallow C assignment is not the duplication mechanism.
Change or free input bytes immediately
after return and verify the context's independent input state. Test embedded
zero bytes, empty/large sequences, malformed encodings, allocator failure, and
unsupported reference-bearing exports. Validate a false-to-true-to-false failure
sequence without a stale description. No test may rely on native struct padding
as a portable serialization format.

## Keep failure boundaries and diagnostics explicit

<a id="embed-05"></a>

**EMBED-05.** A declared application failure is a technically successful result
under [ERROR-01](diagnostics.md#error-01). An exported operation returning such
a value may complete or yield normally. A technical failure terminates only
the affected component execution context and returns the failed host outcome
after supported cleanup; the library must not call process exit, silently
restart the context, or continue its failed computation.

The host interface provides a diagnostic with explicit ownership and release
rules. It may use an independently owned diagnostic copy or documented immutable
messages, but it cannot lend bytes whose context/library lifetime has already
ended. Report the failed component and its Ama execution path; nested adapters
may add their boundary context without pretending that a C helper is another
ordinary Ama return link. Preserve the primary failure while reporting cleanup
failures under [ERROR-02](diagnostics.md#error-02) and PROTO-10.

A supervisor in another, still healthy context may explicitly translate a child
context's failed outcome into its own **declared application failure**, after
the child call returns. This is an isolation/host policy, not an Ama exception
handler and not a way to recover from the supervisor's own runtime failure.
Alternatively the adapter can report a technical failure to its outer context,
which then fails under the ordinary rules. Document which policy an adapter
implements; it must not infer one from diagnostic text or a result's name.

When an adapter propagates a technical status, its `AmaStatus` still follows
[ERROR-04](diagnostics.md#error-04)'s program-lifetime immutable-message rule.
It must use a suitable message owned by the outer program rather than a pointer
into a child library that can unload. A detailed child diagnostic may instead
be copied into an independently owned host report or declared outer application
failure description, subject to that interface's lifetime. Allocation failure
while creating that report is a new outer technical failure, not a successful
application result.

After a child technical failure, a retained adapter must mark that child failed
and permit only its diagnostics/cleanup. A supervisor policy that keeps the
outer context alive must define an explicit reset/recreation operation or a
terminal adapter state; later ordinary steps must not silently resume or
restart the failed child.

**Example: standalone C host.** The following is schematic C for one generated
interface, with a text request and owned response containing one failure value.
The actual header defines its types, matching release functions and error
details; these spellings are not fixed ABI symbols.

```c
#include "validation_component.h"

int validate_document(const unsigned char *bytes, size_t length)
{
    ValidationContext context = VALIDATION_EMPTY_CONTEXT;
    ValidationInstance instance = VALIDATION_EMPTY_INSTANCE;
    ValidationReply reply = VALIDATION_EMPTY_REPLY;
    int application_result = 1;

    ValidationCreation creation = validation_context_create(&context);
    if (!creation.created) {
        /* Creation diagnostics are available without a valid context handle. */
        validation_report_creation_diagnostic(&creation);
        validation_creation_release(&creation);
        return 1;
    }
    validation_creation_release(&creation);

    creation = validation_instance_create(context, &instance);
    if (!creation.created) {
        validation_report_creation_diagnostic(&creation);
        validation_creation_release(&creation);
        goto cleanup;
    }
    validation_creation_release(&creation);

    ValidationCall call = validation_enter(context, instance, bytes, length, &reply);
    if (call.admission == VALIDATION_REJECTED)
        goto cleanup; /* No new reply was published. */
    if (call.outcome == VALIDATION_FAILED) {
        validation_report_owned_diagnostic(context);
        goto cleanup;
    }
    if (call.outcome == VALIDATION_YIELDED) {
        /* This host elects to cancel rather than resume this computation. */
        goto cleanup;
    }
    application_result = reply.failure.present ? 1 : 0;

cleanup:
    validation_reply_release(&reply);
    /* Context cleanup also owns any still-live exported instance. */
    if (!validation_context_dispose(context))
        application_result = 1;
    return application_result;
}
```

The library reports failure to its caller; this host chooses its own integer
return convention. A host that supports yield instead examines the yielded
snapshot, releases it, and explicitly resumes when ready. It must not assume
that every yield means "try again immediately" or execute a hidden busy loop.

## Link, identify, and unload components safely

<a id="embed-06"></a>

**EMBED-06.** Derive collision-checked externally linkable names from the complete
component identity, export identity, and supported ABI binding. Names must not
depend on discovery/link order or merely on a module basename. Apply isolation
to generated public symbols, any required external runtime/helper symbols, and
native companion contributions. Private functions may have internal linkage
where valid; platform symbol visibility alone must not permit unintended
interposition. Public type declarations must remain component-qualified and
independently usable when several generated headers appear in one C translation
unit. Any deliberately shared support declarations require identical compatible
guarded definitions; this does not expose private native `AmaStatus` definitions.

The same Ama logical module included in two compiled components does not merge
their pools, constants, runtime registrations, or nominal runtime identities.
Components remain separate execution domains even if source paths match.
Exchange declared transport values, not private slot identities. A component's
identity/export configuration and ABI assumptions must match its public header
and manifest; diagnose detectable mismatches before execution. A verified build
uses the selected target C ABI and documented compatible native dependencies,
not the compiler host's data model. This requirement does not promise universal
binary compatibility or require a general dynamic plugin discovery mechanism.

Allocate and release each owned resource with the component/API that owns it;
do not free library buffers with an unrelated caller allocator. If an allocator
is configurable, its explicit contract must cover corresponding allocation,
release and library/context lifetimes. No implementation may assume that two
linked components use interchangeable heaps merely because their C types match.

Unloading is forbidden while a live context, exported instance, retained view,
unreleased buffer/diagnostic, asynchronous completion record, or registered
callback still requires the component's code/data or release operation. Context
disposal must cancel/detach native activity under the explicit async lifetime
protocol; a detached record can outlive the context and therefore still prevent
unload until its native completion/retirement. A diagnostic borrowed from an
immutable library message has the same restriction. Independently owned copies
whose entire contract is independent of that library do not retain it.

These are supported-host lifetime obligations. A supported loader/integration
must retain code pins or equivalent exact ownership until every such dependency
has retired; the caller must keep an externally loaded library resident when it
manages loading itself. This contract cannot intercept or make safe an arbitrary
external operating-system unload such as `dlclose` performed in violation of
those obligations. A component can expose pin/count checks for its supported
loader, but cannot promise safety after another owner forcibly unmaps its code.

**Implementation and tests.** Use exact component-qualified namespace generation
and a build manifest checked by the host integration. Link two libraries whose
Ama modules and native helper basenames match; use both headers together and
exercise independent state. Test stale handles after context reuse, mismatched
headers/manifests, allocator pairing, and disposal with late native completion.
An unload test first proves that no resources require the library, then unloads;
it does not test safety by calling a dangling function pointer after unloading.

## Own a foreign component inside a native module

<a id="embed-07"></a>

**EMBED-07.** A native Ama module may own another compiled component through
that component's public C host interface. This is a narrow exception to the
opaque-payload prohibition in [CABI-05](native-interface.md#cabi-05): the payload
may own opaque **foreign host context/instance resources** governed by this
chapter. It may not retain a handle into its current or an active ancestor
context, an ordinary checked Ama reference, a borrowed Ama view, or an encoded
substitute for any of them. A foreign host handle controls an isolated resource;
it grants no access to the parent's ownership graph.

Ownership of a foreign context is exclusive and forms an acyclic nesting forest.
The baseline adapter creates a fresh child context; copying a host token does
not create another owner. An independently specified ownership-transfer service
would have to validate quiescence and preserve this invariant; this facility
does not implicitly adopt an existing shared or ancestor context. Disposal of
the outer resource therefore cannot recursively dispose its own context through
a cycle of child owners.

The adapter's constructor may create its child context, initialize child
constants, and create explicitly selected child instances under EMBED-02. It
must not execute their Ama bodies during construction. Its destructor disposes
the resources it successfully acquired exactly once, including partially
initialized and yielded children, and respects deferred-native-activity/unload
constraints. A stateless adapter may instead create and dispose an isolated
child within a single native entry, when its contract needs no retained state.

A baseline resource-owning adapter does not declare the optional native `=`
protocol and is therefore noncopyable under the normal capability rules.
Neither ordinary copying nor `%` copying can duplicate its opaque child handles.
It must never obtain accidental copyability from a raw struct assignment or
two owners of the same destructor responsibility. Any explicit clone support
requires a separate complete independent child-state reconstruction contract;
it is not implied by this embedding facility. The parent and child cannot share
mutable Ama values/references through the payload; use EMBED-04's transports.

**Example native resource.** These illustrative declarations are internal to
one native companion, using the public foreign header:

```c
#include "validation_component.h"

typedef enum {
    AMA_NATIVE(child_ready),
    AMA_NATIVE(child_yielded),
    AMA_NATIVE(child_failed)
} AMA_NATIVE(ChildPhase);

typedef struct {
    ValidationContext child_context;
    ValidationInstance child_instance;
    AMA_NATIVE(ChildPhase) child_phase;
} AMA_NATIVE(State);
```

Its `.abi` binds `+` and `-` to partial-construction-safe creation and disposal
hooks. It omits `=`. The stored handle is an exported foreign control resource,
not an Ama field pointer. Data passed to or returned from the child still uses
typed independent transport. The parent keeps any persistent application state
in its own declared fields rather than hiding parent references in this struct.

**Acceptance.** Exercise constructor failure after child context creation,
first/repeated entry, yielded child disposal, outer technical failure cleanup,
and exactly-once child resource release. Attempt both ordinary and `%` copying
and reject them statically when `=` is absent. Reject same-context or active
ancestor reentry even if the attempted call uses a different exported instance.

## Return from nested calls before yielding

<a id="embed-08"></a>

**EMBED-08.** A native helper may synchronously invoke the public host API of a
different, isolated component context while the outer native entry executes.
This explicit component boundary may use ordinary C call frames under
[STACK-02](compiler.md#stack-02). Inside either component, ordinary Ama calls
still use generated explicit control transfers. No helper may invoke a private
dispatcher, retain a C stack frame as a continuation, or bypass EMBED-03's
context gate to call the current or any active ancestor context.

Child completion, yield, or technical failure returns normally through the C
host API. Before the outer Ama program reaches `%`, every intervening native
C helper frame must have returned. Child yield is not an implicit outer yield,
transitive await, background scheduling, or permission to spin-resume the child.
The adapter records the child resource's host state explicitly; an outer Ama
module decides whether to yield, complete, report a result, or schedule another
step. A later outer entry/resume explicitly invokes the adapter again, which
resumes the same child only if its stored host state says a saved position exists.
Resumption supplies no fresh child inputs; a requested restart is a separate
entry operation with the usual reset semantics.

Retained completion callbacks created by Ama native operations follow
[ASYNC-05](../language/async-and-parallelism.md#async-05):
they may deliver their own native completion records, not execute Ama or retain
borrowed parent/child views. The outer host/scheduler observes readiness and
chooses when to invoke another step. A yielded child by itself creates neither
an event registration nor an implicit wait for the parent.

A top-level event handler in an external C application may itself be the explicit
host of a quiescent library context, provided the normal admission/lifetime rules
hold and no active Ama/native call is being reentered. This is different from an
Ama operation's retained completion callback, which only records an event. The
callback restriction is an authoring contract; the API need not infer arbitrary
C callback provenance. Its checked gate must still reject actual reentry into
an active context regardless of which C function attempted it.

**Example: one adapter step.** The native entry's ordinary helper chooses an
explicit child operation. `copy_snapshot_to_outer_results` constructs declared
outer results through generated typed access without releasing the child's
snapshot; the caller releases that snapshot exactly once after processing.
`report_child_failure` implements the adapter's documented EMBED-05 policy.
For this example, the adapter's `document` input is consumed only on fresh child
entry. While a child is yielded, another step resumes that existing request and
ignores the newly supplied document input; it never rebinds child inputs or
rereads the original caller's buffer. The child's independently owned input
state was established by its earlier accepted entry. This behavior belongs to
the adapter's documented interface, not an implicit rule for all adapters.
All helper names and signatures below are schematic:

```c
ValidationCall call;
ValidationReply reply = VALIDATION_EMPTY_REPLY;

/* A failed child requires the adapter's explicit cleanup/reset policy. */
if (state->child_phase == AMA_NATIVE(child_failed))
    return report_terminal_child_failure();

if (state->child_phase == AMA_NATIVE(child_yielded))
    call = validation_resume(state->child_context, state->child_instance, &reply);
else
    call = validation_enter(state->child_context, state->child_instance,
                            input_bytes, input_length, &reply);

if (call.admission == VALIDATION_REJECTED) {
    report_adapter_contract_failure();
} else if (call.outcome == VALIDATION_FAILED) {
    state->child_phase = AMA_NATIVE(child_failed);
    report_child_failure(state->child_context);
} else {
    bool yielded = call.outcome == VALIDATION_YIELDED;
    state->child_phase = yielded ? AMA_NATIVE(child_yielded) : AMA_NATIVE(child_ready);
    copy_snapshot_to_outer_results(&reply, yielded);
}
validation_reply_release(&reply);
/* Return from this helper and native entry; do not execute outer '%' here. */
```

**Example Ama caller fragment.** Assume `embedded validator` is a declared owned
adapter, `document` is a declared input value, its typed results are `phase` and
one `failure` value, and `report failure` accepts that complete failure value.
The phase reports the adapter's documented completed/yielded state. These are
illustrative library interfaces, not reserved names or automatic runtime hooks:

```text
embedded validator !
 document < document
 phase > current phase
 failure > child failure

child failure
?
 report failure !
  < child failure
```

If the child yields, the adapter returns a yielded phase while its native entry
finishes normally. An outer state machine may then execute its own `%`, keeping
its declared state and the owned adapter resource alive. On the next explicit
outer resume it calls the adapter again; that new C call resumes the child.
The saved child context belongs to the adapter, the saved outer position belongs
to the outer Ama instance, and neither contains a suspended C helper frame.

**Invariant and tests.** Instrument C helper entry/exit and verify that no helper
activation survives a returned child yield or an outer `%`. Exercise several
child yields, explicit outer yielding, child completion followed by a fresh
entry, deliberate child failure under both documented supervisor policies, and
conforming callback event delivery and rejected active-ancestor reentry. Confirm
that child yield alone does not resume either component, enqueue hidden work,
or mutate outer scheduler state.
