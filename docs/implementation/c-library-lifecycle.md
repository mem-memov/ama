# Explicit C component lifecycle

The checked C header has generated implementations for context creation, exported
instance construction, diagnostics, and disposal. Factories initialize constants
and declared fields, including native resources, without entering an Ama body.
The public consumer needs only its generated header. Private adapters use the
same descriptors and stable-slot runtime as ordinary generated projects.

This implements [EMBED-02](../../spec/implementation/c-library.md#embed-02),
[EMBED-03](../../spec/implementation/c-library.md#embed-03) and the lifecycle
portion of [EMBED-04](../../spec/implementation/c-library.md#embed-04).
[Typed transport](c-library-transport.md) provides input recipes and retained
reads; [entry/resume execution](c-library-execution.md) preserves the same
checked root/context ownership. Artifact packaging and code lifetime management
remain T128. The lifecycle generator is an internal
composition interface, not an additional CLI build target.

## Component setup and explicit ownership

`library_lifecycle_write` accepts the resolved export model and the exact
assembly descriptor ordinal for every export in declaration order. It rejects
an incomplete map, zero ordinal or unsupported C ABI before writing definitions.
The project builder owns this mapping; no module name or generated hash is
interpreted as an ordinal. The generated definitions follow the matching public
header and private project assembly in one translation unit.

A C11 `call_once` initializes a stable component service and copies immutable
project descriptors. It allocates no Ama context or instance and executes no
lifecycle hook. Calls use the ordinary C allocator. The immutable component label
is added to synchronous technical-failure reports before the runtime prints the
intact Ama source path. No callback runs while a service metadata lock is held.

Each successful context factory initializes independent pools, descriptors and
shared constants. Each instance factory creates one explicit default-initialized
root in that context. Several roots can coexist. Neither operation invokes entry
or resumes a continuation. Interface reset belongs to the later explicit entry.

## Checked controls and admission

A public context contains a service owner identity and nonwrapping core context
identity. An instance adds a context-local nonwrapping root identity. These are
numeric controls, never addresses to dereference. The runtime first compares the
owner with its known stable service address, then looks up live context
membership, acquires the context executor gate and resolves the root mapping.
The actual pool handle stays private. Equal local slot/generation or root numbers
in different contexts grant no access to the other context.

A short atomic service gate protects context membership. A core lease pins the
host record and excludes all other operations on that context. The lock order is
service membership then core membership; neither gate encloses native hooks or
allocation. A different thread or reentrant constructor cannot bypass admission
by selecting another root. Independent contexts retain separate executor gates.

Context identities use the runtime image's monotone allocator; disposing and
recreating a service/context does not reset it. Root IDs are issued at most once
within their context. `UINT64_MAX` can be issued once; subsequent construction
fails technically rather than wrapping. Public inline wrappers first compare the
complete [C ABI identity and layout](c-library-header.md). Unload/reload history
is part of T128's code-lifetime boundary, not a promise made by a raw service
address on its own.

Context and root membership searches compare exact issued IDs. With C live
contexts, R roots in the selected context and E configured exports, admission
takes O(C + R + E) in the largest factory/disposal composition; there are no
guessed limits or retry-based identity decisions. Metadata gates cover finite
list operations, while native work runs after the relevant context is pinned.

## Failure and cleanup

A failed context factory publishes no control and returns its cause directly.
An admitted root-construction failure releases its successfully constructed
prefix, terminates the affected context and cleans its earlier roots/constants.
The failed context keeps only a diagnostic record until explicit disposal;
further construction is refused. Other contexts remain usable.

Availability refusals preserve controls and retained state. Accepted destructor
errors consume their target and fail the context, including when native code
returns the numeric `AMA_BUSY` code. Explicit admission booleans distinguish
that technical failure from a busy preflight. Failure to allocate deletion
preflight scratch is also technical, not an availability refusal.

Disposal never executes saved code. A root consumes its native payload and
owned children once. Context cleanup consumes surviving successful roots in
reverse creation order, then constants in reverse initialization order, under
[LIFE-05](../../spec/implementation/native-interface.md#life-05). Cleanup
continues after an error and preserves the primary cause. Context membership is
removed before native cleanup callbacks. If core retirement itself was not
admitted, host membership/control are restored; an accepted cleanup error must
never restore a dead identity.

Diagnostics use immutable program-lifetime native messages under
[ERROR-04](../../spec/implementation/diagnostics.md#error-04). A missing message
uses the immutable component label and retains the numeric cause. Extraction
returns a separate public value that survives context disposal. Release empties
that value and allocates nothing. The host must keep component code loaded until
all such diagnostics are released; a C shallow assignment does not create a
second releasing owner. Full path and secondary cleanup reports are emitted
synchronously before the runtime destroys their source metadata.

## Executable evidence

`make -C tests/unit/runtime/library` composes address-sensitive native resources
with the real core registry, stable pools, cleanup and public-control admission.
It verifies equal local IDs across contexts, stale and foreign controls,
constructor reentry, overlapping C threads, held leases, retained yielded state,
last-ID exhaustion, pre-retirement rejection and exactly-once cleanup. It measures
successful allocation counts and fails every allocation of context creation,
root construction and deletion; every attempt must release all counted storage.
Native destructor Busy is tested independently of executor Busy.

`make -C tests/unit/library/lifecycle` generates a real Ama project assembly,
public header, ABI receiver and lifecycle definitions. An independent C
translation unit first compiles with only the public header, then links against
that assembly. It creates two contexts/roots, rejects cross-context and stale
controls, queries/releases diagnostics, disposes roots/contexts, injects factory
and admitted-construction allocation failures and checks diagnostic bytes after
context disposal. No test stub supplies a lifecycle implementation.
