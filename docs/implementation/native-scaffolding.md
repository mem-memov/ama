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

All file bytes are prepared before publication. Destinations are preflighted,
then staged alongside their targets. Replacement uses per-file atomic rename;
new files use exclusive hard-link publication. Existing files have rollback
backups. Ordinary publication failure restores prior contents; a failed rollback
reports retained backups. Empty directories can remain after failure. This is not
a power-loss-atomic multi-file transaction and assumes an exclusively controlled,
stable destination filesystem. No unrequested files are deleted.

## Evidence and limits

[Component tests](../../tests/unit/native/scaffold/main.c) compose the actual
frontend, typed plan, identity registry, emitters and filesystem publisher. GCC
checks a generated header as the first project include, repeat inclusion, macro
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
