# Native authoring and generated declarations

The native-scaffolding component borrows the shared validated frontend and owns
its declaration model, collision-checked identities and complete output bytes.
Destroy the scaffold before the frontend. Build once and publish explicitly.
Allocation failure retains a diagnostic and never publishes partial model data.

## Authoring commands

```
ama --native ROOT .module --out OUTPUT --sdk /absolute/path/to/ama/src
ama --native ROOT .module --out OUTPUT --sdk /absolute/path/to/ama/src --create
```

The first form uses ordinary strict checking. The second explicitly proposes
missing companions for the selected `@C` module; other required modules remain
strictly checked. A missing ABI starts with mandatory `+ initialize` and
`- destroy` only. `--protocols '|=?[]'` explicitly requests optional hooks during
creation; no capability is inferred from usage. Existing contracts are used by
default. Proposed interfaces are marked authoring-only and are not evidence that
handwritten C exists, compiles, or implements its contract. `--check` still requires
all companions. `--overwrite` requires `--create` and explicitly permits replacing
existing outputs. Ordinary generation preserves handwritten companions.

## Declaration algebra

For every resolved nominal type T, the generator registers distinct qualified
identities for its read/write views, reference/list adapters and input/output
aggregates. A member identity contains its module path, source name and category.
The collision registry compares full keys, so equal emitted identifiers imply
identical qualified identities or generation fails. Sorting by exact logical path
makes declarations independent of directory enumeration.

All tags are declared before definitions. An owning field points to a typed
operation-scoped child view; a reference or list view points to an incomplete,
module-qualified adapter storage type. Thus legal reference cycles impose no
C definition-order cycle. These are ABI views, not the future instance-pool
layout. No runtime storage, pool implementation, entry/hook body or companion
`.c` is included in a declaration header.

`ama-native-types.h` includes standard types, SDK services and each required
native representation header. A native representation defines the tagged
`struct AMA_NATIVE(State)`; its forward typedef is generated. Native payloads
contain native-owned data, never hidden persistent Ama references. Native headers
may include ordinary C dependency headers. The generated interface header directly
includes the common type header and supplies entry function types and hook
prototypes. It can be the first project include in a translation unit.

## Scoped names and skeletons

Each module has explicit `.begin.h` and `.end.h` contexts. The interface opens and
closes its own context; the C skeleton opens a separate body context.
`AMA_NATIVE(name)` qualifies native helper names. `AMA_INPUT(n)`, `AMA_OUTPUT(n)`
and `AMA_FIELD(n)` select hashed members by zero-based role or declaration ordinal;
comments preserve source names, including Unicode. All context macros are removed
on exit. Do not nest naming contexts. Every definition uses the same registered
identity as its declaration. Source `#line` mappings retain original Ama paths.

The C skeleton directly includes its generated interface. Native entry uses the
mandatory inline attribute; lifecycle hooks have the exact checked ABI signatures.
Unimplemented bodies return explicit failure, never fictional success. A separate
lifecycle test skeleton calls construction/destruction with `assert`; it becomes
executable after the author implements the hooks. It is not run as a passing test
of an unimplemented native module.

Physical include paths containing quotes, backslashes or control bytes cannot be
represented in the selected C header-name form and are diagnosed. Unicode and
spaces are supported. Output-directory components must be ordinary directories.

## Publication boundary

The [publication phase contract](native-publication.md) describes T139's narrow
filesystem children, resource ownership, exact completion point and independent/
real-parent failure checks.

All file bytes are prepared before publication. Destinations are preflighted,
then staged alongside their targets. Replacement uses per-file atomic rename;
new files use exclusive hard-link publication. Existing files have rollback
backups. Ordinary publication failure restores prior contents; a failed rollback
reports retained backups. Empty directories can remain after failure. This is not
a power-loss-atomic multi-file transaction and assumes an exclusively controlled,
stable destination filesystem. No unrequested files are deleted.

## Evidence and limits

[Component tests](../../tests/unit/native/scaffold/declarations/main.c) compose the actual
frontend, typed plan, identity registry, emitters and filesystem publisher. GCC
checks every required native interface header as the first project include, repeat inclusion, macro
cleanup, transitive native types, reference cycles and all storage forms, then
checks the untouched native scaffold. Every model allocation is fault-injected.
These checks precede the execution backend; final assembly composition remains a
backend acceptance obligation.

## Editor and syntax-check configuration

`compile_commands.json` uses the standard `directory`, `file`, `arguments` schema.
Arguments are JSON strings, never a shell command; quotes, backslashes, control
bytes and UTF-8 are preserved without shell evaluation. Entries cover required
native companions and selected C/test templates. SDK/output include paths are
absolute; C11, diagnostic flags, assertions and syntax-only mode are explicit.
`--cc PROGRAM` selects the compiler; repeated `--c-flag ARG` appends exact arguments.
No forced include or implicit current-directory include is required. The SDK
header enforces the verified GCC target. User flags are explicit configuration,
not inferred from local files. A test uses Python's standard JSON reader and runs
every exported command; the test framework and fixtures remain C/Make.

## Output ownership

The versioned `.ama-native-outputs` manifest records each generated basename and
its SHA-256 digest. The manifest itself is not recursively hashed. Records are
unique flat names with exact lowercase hex digests; malformed, duplicate or unsafe
records are errors, including with overwrite requested. This is an explicit local
ownership ledger, not authentication against hostile filesystem changes.

Normal refresh replaces an existing generated file only when its current digest
equals its recorded digest. Unknown files and edited owned outputs cause a conflict
before any destination contents change. Only explicit overwrite permits replacing
them. Handwritten companions never enter the ownership ledger: `--create` fills
missing files and preserves existing ones; `--create --overwrite` permits replacement.
Prior records outside the current generated set are retained without deleting files.

An exclusive `.ama-native.lock` serializes publication. A retained lock is an
explicit conflict; its age never licenses removal. The user can inspect/remove a
crash-retained lock before retrying. A scaffold publishes at most once, including
failed attempts. Allocation-failure tests cover manifest loading, ownership checks,
staging and rollback-path setup; additional cases cover refresh, handwritten files,
edited outputs, symlinks, malformed manifests and retained locks.

## Exact hook roles and composition acceptance

The emitted native role names `State`, `EntrySignature` and `entry` belong to the
C adapter convention. A conflicting ABI helper is diagnosed before publication.
Two operations may reuse one helper when their C signatures are identical: the
mandatory constructor and destructor both take `State *` and return `AmaStatus`.
Such a helper is declared/defined once; each protocol still retains its binding.
Reusing one name for incompatible signatures is diagnosed without parsing C.

Acceptance also composes two emitted native bodies with identical local helper
names into one C unit, uses typed parameter/result payload views to transfer a
value, calls both inline entries directly, and runs the harness. It checks isolated
headers, Unicode paths/names, paths with spaces, quoted compiler arguments, exported
commands and the real authoring/check/refresh CLI. [Ownership tests](../../tests/unit/native/scaffold/ownership/main.c)
are a separate leaf. The [final Ama assembly](project-emission.md) has its own
actual generated-execution tests; this native-only composition certifies only
declaration/namespace agreement.

## Bounds and failure invariants

Let T be the number of nominal types, F declared fields, N manifest/output records
and B processed bytes. Every traversal is over a finite input list or byte range;
hook comparisons are bounded by the six ABI slots. Forward declarations precede
every typed pointer use, so cycles in reference graphs require no recursive
layout expansion. Qualified-identity registration compares full encodings after
hashing and rejects collisions rather than relying on probabilistic uniqueness.

Model construction uses insertion ordering and linear identity/type lookup; its
comparison count is bounded quadratically in T+F, multiplied by compared key
lengths. Ownership uses quadratic record/name lookup plus linear byte loading and
hashing. Retained memory is linear in emitted/loaded bytes and model/key records.
There is no arbitrary graph-depth, timeout or retry heuristic.

Before publication all outputs are complete immutable byte strings. Before any
destination replacement, every conflict is checked and every stage/backup exists.
Each successful publication step advances one artifact's flag. Ordinary failure
restores the flagged prefix and removes staging; failure to restore reports that
condition and retains recovery evidence. Hash checks protect author edits under
the stated stable-filesystem assumption; they do not defend against a hostile
process changing destinations outside the lock protocol.

Keep generated output outside indexed module roots, or pass an exact `--exclude`
for its location. Output discovery never guesses exclusions from directory names.

The manifest also records a scope digest of the exact absolute selected Ama source
path. Different entries/projects use separate output directories; switching a
valid existing scope requires explicit overwrite. This prevents generating one
module's smaller closure over another module's common type header by accident.
Malformed scope syntax remains an error with overwrite.


T148 adds Doxygen contracts for every retained native model/emitter/namespace/
ABI/toolchain boundary, reusing T139's independently checked frontend-free
publication phases. The model/identity/source owners borrow the complete frontend;
publication borrows only the IO/error/ownership views documented in
[native publication](native-publication.md). Scope/digest records are explicit
refresh authority under the stable-filesystem/lock assumptions, not generated
ownership inferred from a legal basename. No power-loss transaction or protection
against an unrelated writer outside the lock protocol is claimed.

The six protocol metadata records are independent of C definition deduplication.
T148's regression found that sharing one compatible C helper for `+` and `-`
emitted only `+`, making the generated `.abi` fail its own mandatory-slot parser.
The generator now emits every present binding while still declaring/defining the
compatible C function once. The regression parses the actual staged ABI bytes and
checks both exact bindings; it failed before the correction. The signature writer
also now receives only stream, protocol operation and original function span;
its unused type-model argument and the matching private helper argument are gone.

Supported configuration strings are UTF-8, and path preparation rejects C header
name syntax hazards rather than inventing an escaping convention. C signatures,
header include order, nominal view constness and namespace cleanup are checked by
actual isolated/composed C compilation. Those checks cannot prove arbitrary native
ownership, pointer lifetime or callback purity: trusted authored C must satisfy
its bounded-borrow/resource contracts. Compiled-component exports and cross-context
ABI/configuration compatibility remain their own TODO work.

## Machine-readable authoring identities

Declaration generation also emits `native-model.json` (`ama-native-model-1`).
It maps each native logical module to its exact namespace, parameter/result C
names and full framed identity keys, together with interface/context header
basenames. The emitter traverses the same deterministic scaffold list and uses
the same registered identities as the declarations, so every record has one
exact declaration counterpart; no filename/hash inference is needed. Emission
is linear in the serialized identity bytes and propagates output failures through
the ordinary owned-artifact publisher. Empty native sets produce an empty array.
The [CLI examples](c-library-examples.md#native-authoring-without-guessing-generated-names)
exercise bootstrap/final-record equality and explain the format. The declaration
test independently compiles the mapped interface and typed views.
