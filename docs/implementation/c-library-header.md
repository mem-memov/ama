# Public C consumer header

The checked export model now generates a self-contained C11 header. It declares
opaque host control identities, typed readonly input views, opaque owned result
snapshots and their ownership operations. It also generates the actual primitive
ABI receiver used to check a consumer before any structure-based C call.
The [lifecycle implementations](c-library-lifecycle.md) provide explicit factories
and disposal. Execution, snapshots and artifact packaging follow in T126–T128.
The header alone is not a compiled Ama component.

```sh
make
./ama --library-header tests/integration/library-exports/project tests/integration/library-exports/component.exports --standard-root stdlib/modules > /tmp/ama-component.h
```

The command uses the [explicit configuration](c-library-exports.md), writes only
header bytes to stdout, and reports errors on stderr. It accepts the same source
root/exclusion options as library checking. It does not accept executable
compilation/output-directory flags. No Ama body or native C compiler runs while
generating declarations.

## Supported C ABI

The initial consumer profile is GNU-compatible x86-64 Linux ELF, LP64, with the
platform's System V C calling convention. The explicit target descriptions
`x86_64-linux-gnu` and `x86_64-pc-linux-gnu` select this profile. A metadata-only
check can retain another declared target; header/bootstrap generation rejects
it with the original target-record location before writing partial C output.
Additional targets need their own explicit checked profile.

The header includes only `stddef.h`, `stdint.h` and `limits.h`. It requires C11,
eight-bit octets and the profile's 64-bit pointer/size model. Compiler target
macros check the declared platform. A GNU-compatible function-type assertion
compares the compiler's default convention with explicit `sysv_abi`; for example,
`-mabi=ms` is rejected during C compilation before it can corrupt the bootstrap
call itself. Structure packing differences are checked separately at runtime.
No native-authoring header, unqualified `AmaStatus`, pool layout or dispatcher
record appears in the public declarations.

This implements [EMBED-06](../../spec/implementation/c-library.md#embed-06):
version, target and C calling convention are prerequisites of calling across the
boundary. A descriptor cannot safely discover an incompatible fundamental C
calling convention by making a call through that incompatible convention.

## Find names without inventing them

`--library-check` reports the component, export and schema identity names. These
are complete, component-qualified C prefixes. Appending the documented role
suffix yields the corresponding public type/function; no consumer scans an Ama
module's basename or reconstructs internal names from filesystem separators.

For a component prefix P, the public control/result types are:

| Type | Contract |
|---|---|
| `P_Context` | Copyable opaque numeric control identity. All-zero is empty. Disposal invalidates every copy; a handle copy does not retain its context. |
| `P_Instance` | Copyable opaque identity bound to one export and its originating context. No Ama address or local handle triple is exposed. |
| `P_Bytes` | Readonly counted octets, without implicit termination or transfer. |
| `P_Diagnostic` | One owned or component-pinned immutable diagnostic view. Initialize empty and release once; message bytes remain valid until release, including after context disposal. |
| `P_ApiResult` | Separate API code plus any owned diagnostic returned by the operation. A function return transfers the diagnostic owner; C assignment is not cloning. |

The uppercase component prefix supplies ABI edition, required-edition,
component-ID, target, calling-convention and result/outcome constants. Metadata
strings are emitted as exact octal octets: quotes, backslashes, trigraph-sensitive
sequences and non-ASCII UTF-8 cannot inject C tokens or change their byte values
under execution-charset conversion.

The API code channel distinguishes OK, invalid request, busy, refused,
ABI mismatch and technical operation failure. Accepted execution additionally
writes exactly completed/yielded/failed to its outcome destination. API rejection
leaves that outcome unchanged. API OK with execution failed means a technical
Ama execution failure, not an ordinary application failure or a fourth outcome.
A context-factory failure returns its diagnostic directly without requiring an
unpublished context handle, under
[EMBED-02](../../spec/implementation/c-library.md#embed-02).

## Transport values and ownership

For each explicit schema prefix S, the header declares `S_Borrowed` and opaque
`S_Value`. A value owner is an `S_Value *` initialized NULL. It is independent
of the source instance/context; its contents are exposed only through a readonly
view. A shallow pointer copy does not create another releasing owner.

| Profile | Readonly borrowed view |
|---|---|
| Integer | Canonical 0/1 negative flag and counted big-endian magnitude. Zero is one zero octet with flag zero; nonzero magnitudes have no leading zero. |
| Text | Counted UTF-8 data, including embedded NUL. |
| Condition | One byte with value 0 or 1. |
| Failure | One structure with a 0/1 `failed` member and counted UTF-8 `description`; false requires an empty description. |
| List | Counted array of the explicitly declared scalar borrowed-element type. |

Failure therefore remains one typed result under
[ERROR-01](../../spec/implementation/diagnostics.md#error-01). The result does not
become separate Boolean and reason outputs. Its view contains both parts, and
its explicit clone/release operations govern the whole value.

`S_view` borrows data until the owning snapshot is released; a failed view leaves
the destination unchanged. `S_clone` prepares an independent releasing owner into
an initialized-NULL destination and preserves both operands on failure.
`S_release` consumes the sole pointer and leaves it NULL; releasing initialized
NULL is a no-op. These are the ownership contracts of
[EMBED-04](../../spec/implementation/c-library.md#embed-04).
Diagnostic/result release similarly empties the releasing owner. Component code
must remain loaded throughout calls and every still-owned resource.

An export always has an explicit instance factory. Only configured entry,
resume and retained-read operations are declared. Entry accepts typed borrowed
parameter pointers in interface declaration order; NULL selects the ordinary
parameter default. Resume/read accept no new parameters. Optional result
destination pointers may be NULL to skip a snapshot; otherwise their pointed-to
owning pointers must be initialized NULL before admission. Owned output locations
must be distinct; aliases are invalid requests, preventing overwritten releasing
owners. Declared field names
and role ordinals come from the checked model, not a second source parser.

Context creation constructs constants/services but no executable root. Instance
creation initializes one explicit root without running its body. A later entry
resets its interface even on its first call and preserves body state. Quiescent
disposal invalidates controls without resuming saved code. Reentrant/concurrent
access, snapshot preparation and failed-context restrictions remain governed by
[EMBED-03](../../spec/implementation/c-library.md#embed-03).

## Check layout before structure calls

Every public inline wrapper first calls `P_abi_check`. This uses one primitive
bootstrap whose arguments/return are fixed C integers and valid bounded C
borrows. The receiver compares:

1. The exact supported ABI edition.
2. The complete length-framed component ABI key, compared byte for byte. A hash
   selects candidate C names.
3. Descriptor counts and nonnull bounded input pointers.
4. Every public size, alignment and field offset, including each concrete
   borrowed schema, diagnostic and result structure.

Repeated header inclusion also compares the complete key through exact numeric
word macros plus its length. Two independent headers with a colliding candidate
namespace cannot silently share an include guard. A single compile-time assertion
rejects their unequal keys. The runtime bootstrap compares the full original key
too, covering independently compiled consumers where include guards never meet.

Consumer and receiver descriptors are emitted from the same typed schema and
compiled separately under their actual C options. A consumer's packing pragma
therefore changes its facts and causes rejection. The wrapper then returns a
locally constructed ABI-mismatch result; it has not called a foreign function
with a mismatched structure argument/return.

`library_abi_write` emits the receiver definition after its public header has
been included. It performs bounded integer/byte comparisons, allocates nothing
and touches no Ama context. Immutable static key/layout arrays avoid per-call
stack copies; verification costs O(key bytes + public layout facts). No cache
assumes an unloaded/reloaded code image still has the old ABI. Both generators check writes and final flush;
unsupported targets write nothing. Their caller owns artifact publication.
Names ending `_implementation` declare the raw thunks needed by these wrappers;
directly calling those thunks bypasses the public ABI-check contract.

The exact profile and nonnull/length C-buffer preconditions remain necessary:
a checked token API cannot make arbitrary invalid caller input pointers safe.
The host also retains code for the complete duration of C calls; the primitive
bootstrap is not permission to unload code during an active C activation.

## Tests and per-file evidence

```sh
make -C tests/unit/library/header
make -C tests/unit/driver
make -C tests/integration/library-exports
make docs-complete
make docs-reproducible
```

`tests/unit/library/header` compiles two independent C translation units with
the headers of two components sharing Ama module names, in both include orders,
without an SDK include path. A consumer uses one failure snapshot, views its
condition/description, explicitly clones it and releases each owner. Negative
consumers reject the wrong requested edition, foreign component identity types,
unavailable operations, private `AmaStatus` and a Microsoft default calling
convention. Unsupported declared targets return their exact configuration site
and produce no partial header/bootstrap.

The test links a real generated primitive ABI receiver in another translation
unit. Fault injection changes edition, identity bytes, layout values/counts,
identity length and null borrows. Each mismatch prevents a structure-based
context stub from running. The matching path reaches that stub. A separately
packed receiver is rejected by an ordinary consumer before the stub call.
This tests actual generated comparison code; the context stub only observes
admission and does not replace a claimed Ama lifecycle implementation.

The same leaf executes a standalone metadata-literal probe with Unicode,
quotes, backslashes, comment delimiters and trigraph-sensitive bytes. Repeated
headers compare byte for byte. `/dev/full` checks both generators' write/flush
failure, and allocation counters verify they do not mutate the model arena.

- `src/library/header/preamble.c`, `values.c`, `snapshots.c`, `exports.c` and
  `lifecycle.c`: independent consumer compilation and interface/ownership negatives.
- `src/library/header/abi.c`: actual receiver execution, matching/rejected paths,
  separately packed translation units, forced cross-registry candidate collisions
  with unequal full keys, and stream-failure checks.
- `src/library/header/identity.c`: repeated identical includes and forced candidate
  collisions between different complete keys, including separate linked consumers.
- `src/library/header/write.c`: complete header composition, literal execution,
  deterministic bytes and source/model ownership checks.
- `src/library/header/target.c`: exact supported-profile checks, negative C
  convention compilation and source-backed unsupported-target rejection.
- `src/library/header/internal.h` and `src/library/model.h`: the same consumers
  plus independent standalone header compilation through the documentation gate.
- Updated driver request/options/dispatch/output/library-command files and test
  leaves: real `--library-header` stdout is copied to a C header and compiled
  by an independent consumer without compiler-private include paths.
- `src/library/configuration/records.c`: typed target-record location capture is
  verified by the unsupported-target case, while the configuration/resolve leaves
  retain their earlier negative and allocation-failure contracts.

New test support/main/Make files are registered per file in `ready.json`; their
actual named C/assert cases and commands are checked through the real leaf
Makefiles. Generated headers/sources are compiled through their generators and
are never edited or registered as maintained source.
