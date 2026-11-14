# Explicit C-library exports

T123 adds an independently checked library-build model. It selects exported
module types, operations and transport schemas explicitly, validates them through
the ordinary typed project, and reports deterministic machine-readable metadata.
It does not compile a library, emit a public consumer header, create a runtime
context, execute Ama or create an executable launcher. Those generators and
runtime boundaries are the subsequent T124–T128 tasks.

```sh
make
./ama --library-check tests/integration/library-exports/project tests/integration/library-exports/component.exports --standard-root stdlib/modules
```

The command writes one JSON object on success. `stage: resolved` means the export
model has been checked; it does not claim that a C compiler has verified a binary
for the declared target. `--compile`, `--out`, `--sdk` and native-authoring flags
are unavailable on this command. Its source roots and exclusions have the same
explicit authority as ordinary checking. No implicit standard root is installed.

The [maintained configuration](../../tests/integration/library-exports/component.exports)
and [interface](../../tests/integration/library-exports/project/main) are runnable
examples. This is an implementation-defined build-input format under
[EMBED-01](../../spec/implementation/c-library.md#embed-01), separate from Ama
source and from a native module's `.abi` protocol table. `spec/` remains the
language authority.

## Read a configuration

Each nonblank, noncomment line is an exact record. Fields use the three-byte
separator ` | `; values have no leading/trailing ASCII space. Names inside a
field retain their separating spaces and Unicode bytes. LF is the physical line
separator; the final LF is optional. NUL, CR, tab and malformed UTF-8 are rejected,
including in comments. A comment begins with `#` in column one. The first
noncomment record is exactly `ama-library | 1`.

```text
ama-library | 1
component | example.component
abi | ama-c-1
target | x86_64-linux-gnu
calling-convention | c
schema | message | text | ..text
schema | outcome | failure | ..failure
export | run | .main | entry,resume,read
input | run | message text | message
output | run | failure | outcome
```

| Record | Meaning |
|---|---|
| `component \| identity` | Required explicit component identity; not an inferred module basename. |
| `abi \| ama-c-1` | Required supported public ABI edition. Unknown editions fail. |
| `target \| target-description` | Required selected target description, retained verbatim. Actual C-target verification belongs to artifact compilation. |
| `calling-convention \| c` | Required supported C calling convention. Other conventions fail. |
| `schema \| identifier \| profile \| argument` | Declare an explicit supported transport; the argument is its exact module path or element-schema identifier. |
| `export \| identifier \| module-path \| operations` | Select an executable nominal type and a nonempty comma-separated set of `entry`, `resume`, `read`. |
| `input \| export \| field-name \| schema` | Map one exact parameter name; requires an advertised entry operation. |
| `output \| export \| field-name \| schema` | Map one exact result name for entry/resume/read. |
| `c-flag \| argument` | Preserve one C argument and its ABI effect; no shell splitting or execution occurs while checking. |
| `link-flag \| argument` | Preserve one explicit linker argument. |
| `link-file \| path` | Capture one existing foreign link input by exact bytes; resolve its path from the command's working directory. |
| `dependency-file \| path` | Track an explicit contributing C/header file without passing it as a separate linker input. |

There is one component/ABI/target/convention record, at least one export, and
unique schema/export identifiers. Public identifiers use ASCII letters followed
by letters, digits or underscores, with an initial letter. Ama module and field
names still use their existing complete Unicode naming rules. Operations cannot
repeat. Metadata, schemas and mappings have no defaults. Unknown records fail.
The final record field is its complete remaining byte string; a C argument may
therefore contain another ` | ` without being split into extra arguments.

Two public export identifiers may deliberately select the same module type.
They remain distinct public exports. Every parameter of an entry export and
every result needs exactly one matching mapping. Private fields cannot be
mapped. A narrower public interface can use an explicit Ama facade module.
A resume-only export accepts no input mappings and does not acquire a saved
position merely by being exported. [EMBED-03](../../spec/implementation/c-library.md#embed-03)
still governs later admission and execution outcomes.

## Select a transport deliberately

[EMBED-04](../../spec/implementation/c-library.md#embed-04) forbids serializing
private payload bytes or moving Ama references across contexts. Edition 1's
checked model supports the following explicit profiles. These representations
are value schemas; the subsequent consumer-header generator supplies their exact
C declarations and the transport implementation supplies the declared codecs.

| Configuration profile | Exact argument | Representation contract |
|---|---|---|
| `integer` | `..integer` | Sign and independently owned big-endian magnitude octets; canonical zero is one zero octet with a false negative flag; no leading zero octets on nonzero values. No narrowing to a host integer. |
| `text` | `..text` | Independently owned, explicitly counted UTF-8 octets, including embedded NUL. No locale conversion or implicit C terminator. |
| `condition` | `..std.condition` | A Boolean value representing false or true. |
| `failure` | `..failure` | One owned record containing its Boolean condition and owned UTF-8 description; false requires an empty description. This remains one Ama result. |
| `list` | Another declared scalar-schema identifier | An independently owned ordered array of values of that element schema. A list of lists is unsupported by this edition. |

For example, `schema | messages | list | message` selects an owning list of the
previously declared text transport. The element schema can be declared later;
unknown elements, list cycles and list-as-element shapes fail. Structured custom
modules, native payloads without a supported profile, references and reference
lists are rejected rather than given an inferred representation. Extending these
profiles requires an actual checked codec contract; names in handwritten C are
never scanned for a matching encoder.

Scalar profiles require their exact standard-root nominal identity, native
implementation and declared clone/condition capabilities, plus literal
construction for integer/text/condition. Failure additionally requires exactly
one owned `description` result of nominal `..text`. A transport declaration is
not permission to cast arbitrary native State storage. Profile implementations
must use their typed standard-library contracts.

The JSON manifest names each profile's representation, namespaced encoder,
decoder, release and clone declarations. These names reserve the checked schema's
codec interface for T124/T126; the resolution command does not claim these C
functions have already been generated. Byte lengths and array lengths use the
selected target's checked bounded-size contract. Construction must publish only
complete independent values and release a prepared prefix on failure. Returned
snapshots remain readable after instance/context disposal; live Ama addresses,
reference handles and saved control storage are outside these representations.

## One closure, explicit inputs

`library_build_resolve` loads the first export and all explicitly required export
and scalar-schema types through `project_load_required`. Registry sharing means
one nominal module has one loaded source record even when selected repeatedly.
Unselected modules are not parsed or exported merely because their files exist.
The ordinary frontend then performs the same syntax, semantic, ownership,
constant and resolved-plan validation as executable checking. Its entry pointer
selects a planning root; checking creates no runtime instance or launcher.

The component input consumer depends on the raw configuration and every loaded
module consumer. Existing module consumers retain source, ABI, native C/header
and module-dependency edges. Explicit link and dependency files add byte-exact file edges and
reuse already captured files by exact key, avoiding duplicate native reads.
C/link flags participate through the configuration bytes. Changes propagate by
the existing least fixed-point dependency calculation: unchanged snapshots stay
clean; changing an exported module, a dependency, configuration or link file
invalidates the component consumer. The stable-filesystem-during-load contract
continues to apply. This stage reads handwritten inputs and writes only to the
caller's metadata stream.

## Identity and bounded algorithms

Names use the existing length-framed, root-aware identity registry and its
full-key collision checks under
[EMBED-06](../../spec/implementation/c-library.md#embed-06). The complete ABI key
contains component identity, ABI/target/convention, explicit schema definitions,
export module identities/operation sets, field mappings with resolved role
ordinals, and ordered C/link settings. Category and field boundaries remain
explicit; concatenating ambiguous display names is never identity comparison.
Component, schema and export keys add their own role and complete nominal type.
Changing a public field's declaration order therefore changes the component key.
Comments, physical configuration paths and blank lines do not change ABI names;
the exact configuration bytes still remain tracked build inputs.

For B configuration bytes and R records, parsing visits each byte a bounded
number of times. Arena ownership makes every partial prefix destructible. Linked
record admission/name checks can take O(R²); the configuration is finite, without
recursion or guessed limits. Schema resolution has two passes: exact scalar
contracts first, then lists pointing only to resolved scalars. It cannot recurse
through a cycle. Mapping validation proves a bijection between required typed
interface fields and matching explicit mapping records. Completeness and
uniqueness together exclude silent arity defaults.

Identity key construction checks element and byte bounds before allocation. Each
registered name owns its complete copied key; the shared temporary key vector
can be reused for the next role. A forced unequal-key digest collision is a hard
diagnostic, retaining the previous declaration. A digest is never sufficient to
merge keys. Metadata serialization escapes strings, checks writes and final
flush, and has no filesystem publication side effects.

## Verification and per-file map

```sh
make -C tests/unit/library
make -C tests/unit/driver
make -C tests/integration/library-exports
make -C tests/unit/documentation
make docs-complete
make docs-reproducible
```

- `src/library/configuration/*.c` and `src/library/model.c`: the configuration
  leaf preserves exact multiword names/argument bytes, partitions malformed,
  duplicate and unsupported records, checks required metadata/UTF-8/NUL/CR/tab,
  and injects every parse allocation failure.
- `src/library/resolve/*.c`, `src/library/project/*.c`, `src/library/model.h` and
  `src/library/internal.h`: the resolution leaf checks disconnected explicit
  roots, no accidental draft export, scalar/list/failure profiles, missing and
  duplicate mappings, references, schema cycles, actual field-order ABI changes,
  forced registry collision and every complete-resolution allocation failure.
  Both headers compile standalone through the per-file documentation gate.
- `src/library/report/manifest.c`: repeated metadata streams compare byte for
  byte; `/dev/full` checks write/flush failure. Real CLI tests check public
  field ordinals, single failure schema, list profile, unsupported flags and
  missing physical configurations.
- `src/compiler/load.c` and `src/compiler/internal.h`: the same closure loader
  serves ordinary frontend tests and explicit multi-export composition. Existing
  executable/authoring callers retain their exact single-entry policy.
- Driver recognition/options/dispatch/output/command files: request and option
  component tests plus the real library CLI test leaf prove the configuration is
  a physical path, separate from a logical entry and executable generation.
- New test C/support files, recursive Makefiles and `.exports`/Ama fixtures are
  mapped to their explicitly named leaf cases in `ready.json`. Test fixtures own
  their files; failed leaves preserve output and successful leaves clean it.
- `tools/documentation/coverage.py` and its maintained scenario fixture now retain
  `.exports` build configurations in the input inventory, independently of old
  debt entries. The C/assert documentation leaf checks the real Python boundary.
