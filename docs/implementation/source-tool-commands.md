# Source tooling from the command line

T152 exposes the existing [editor analysis](editor-analysis.md) and
[identity-aware rename](editor-rename.md) through two explicit commands. These
commands read stable filesystem inputs. They do not execute Ama, invoke native
constructors, generate C or write source documents.

```sh
./ama --editor project .main --offset 12
./ama --editor project .main --module .palette --offset 3
./ama --rename-plan project .main --module .palette --offset 3 --name 'new name'
```

The first positional path is an explicit ordinary source root; the second is the
logical entry used to check the project. Existing `--root`, `--standard-root` and
`--exclude` settings retain their normal driver meaning. Exclusions are the
explicit shared root-relative exclusions used by the driver, not inferred output
folders. `--module` chooses a query source and defaults to the supplied entry.
`--offset` is a canonical nonnegative decimal byte offset, including EOF, with no
sign, padding or whitespace. It is optional for `--editor` and required for
`--rename-plan`. Rename also requires one nonempty `--name`. Repeated source-tool
scalar options, unknown options and source selectors on unrelated commands reject.
The production complete-name validator checks the entire replacement name without
trimming or normalization.

## What an editor query means

`--editor` returns `ama.editor.v1` JSON from an immutable production analysis:

- `state` describes the entry analysis: `complete`, `draft` or `invalid`.
- `diagnostics` preserves structured error/warning codes, prose, primary locations
  and related declarations.
- `highlights` contains exact original token/comment/payload spans and bytes, with
  an explicit completion flag and lexical diagnostic. A comment has no invented
  lexical token category. Other category values are the current `LexKind` ordinal
  from the [lexer interface](../../src/lexer/tokens/tokens.h).
- `signature` exposes independent constant/input/result roles, with an explicit
  availability flag. Private state is not a module interface.
- An optional `point` contains the exact definition, checked operation/binding and
  visible-name completion list. Missing matches are null or unavailable, not
  guesses at adjacent spellings. Intrinsic methods use canonical semantic method
  descriptors and have no fabricated nominal module/declaration.

The entry analysis includes its checked dependency graph. Selecting another
catalog source can expose its lexical bytes without making that source part of the
checked graph: unavailable definitions/signatures remain explicitly absent. The
entry `state` must not be read as certification of an unrelated selected source.
For source-owned constructor visibility, inferred declaration order and foreach
scope, the command calls the same semantic predicates as compilation.

A successful observation may report draft/invalid source; this is useful to an
editor while code is being written. It does not claim that `--check` would pass.
Unavailable sources, resource failures, invalid requests and output failures return
nonzero. A response from a failed process must not be accepted as a completed
observation, even if part of its JSON reached the stream.

## A rename plan is a client transaction, not a filesystem edit

`--rename-plan` checks the complete active workspace, including unreachable
importers, prepares exact declaration-identity edits, then freshly validates the
original inventory/native inputs and the candidate declaration/reference/control
graphs. It returns `ama.rename.v1` only after that validation. Draft contents remain
opaque and do not become rename occurrences. A same-name no-op has an empty file
list.

Each changed file contains its physical path, original device/inode, complete
original/replacement bytes and exact old/new name intervals. The declaration
location is the original checked declaration, not the synthetic `<rename>` source
used to validate the proposed name. Comments, literals, storage suffixes and line
endings retain their bytes.

The command performs no write and holds no edit authority after it returns. Its
validation describes a particular snapshot. Checking only changed documents cannot
certify that other sources or native dependencies stayed unchanged. A client must
hold exclusive authority over the complete input domain while producing and
atomically applying the plan, or use `editor_rename_publish` under its document
transaction authority for fresh validation immediately before application.
Expected identities and complete original bytes prevent overwriting a different
changed document; they do not replace whole-workspace revalidation. Saving/applying
an editor transaction and recovering filesystem edits are client responsibilities.
Directory relocation uses its separate transactional commands under T153.

## Lossless response values

The structured output component is
[`src/tools/output/`](../../src/tools/output/json/json.h). Protocol keys and schema
names are fixed; source data enters only typed value encoders.

All offsets, lengths, positions and numerical identities are **decimal strings**.
This preserves the full C integer range when a client uses JavaScript or another
JSON implementation with floating-point numbers. JSON booleans retain their
ordinary types. Missing observations are null or have an explicit availability
flag, rather than sentinel source names.

A source span contains a physical path, half-open byte offsets and one-based
physical line/byte column. Columns count original UTF-8 bytes, not display cells or
UTF-16 units. Physical names and original source/payload bytes use:

```json
{"utf8": "exact bytes", "hex": "6578616374206279746573"}
```

`utf8` is null for invalid UTF-8; `hex` always preserves the exact bytes. The two
fields describe the same bytes when text is valid. NUL/control characters inside valid text
use JSON escapes. Non-ASCII UTF-8 stays unchanged, and arbitrary binary/physical
names never receive a lossy replacement. Nullable C metadata remains null.

The JSON writer borrows an open byte stream and owns no heap memory. It validates
text before opening its quote, retains the first encoding/write error, and never
closes the caller's stream. Success additionally requires flushing. Trusted static
punctuation/keys are separate from untrusted source values, so a source quote or
newline cannot inject response grammar. Every string loop consumes a byte or
fails; UTF-8 validation and output cost are linear in byte count, with constant
encoder memory. Source query/rename cost and ownership remain the existing core
contracts; the command adds no alternative parser or ranked completion heuristic.

The process is a one-shot filesystem-snapshot client, not an LSP server or an
unsaved-document synchronization layer. No editor plugin, automatic buffer offset
conversion, handwritten C rename, filesystem rename apply or stale-plan validity
is claimed. C clients can retain the existing API owners and implement their own
versioned document service. T115 composes those services with real project builds.

## Evidence

- `make -C tests/unit/tools/output/json`: independent standard JSON decoding of
  all control bytes, Unicode, every octet, full-width identities and actual
  write/flush refusal; malformed text has no lossy fallback.
- `make -C tests/unit/driver`: exact command/option applicability, byte-offset
  parsing and duplicates, ordinary driver regressions, checked Unicode/CRLF
  observations, nominal/intrinsic metadata, related diagnostic spans, invalid
  source/name requests, every measured command allocation failure and read-only
  output refusal. Source/native owners remain alive through diagnostic rendering.
- `make -C tests/unit/editor/rename`: the delegated core's exact stale-source,
  native-dependency, scope-capture and client-publication refusals. The CLI does not
  provide a separate publication path that could bypass those checks.
- `make -C tests/integration/tools/source`: actual command processes query a
  complete project, prepare a three-file constant rename including an unreachable
  importer, preserve opaque draft/C bytes, refuse invalid names without partial
  plans, and return an empty no-op plan. The C harness controls execution; Python's
  standard JSON decoder independently verifies the wire observations.

Per-file cases, header checks and final task evidence belong in
[ready.json](documentation/ready.json) and [TODO.md](../../TODO.md).

## Static and captured source observations

T192 exposes named static inputs/results/aliases with source declaration locations,
static category and owning module. It invents no runtime position for them.
An ordinary source field used under several concrete captures retains its original
shape and source role; its concrete Type is null when contexts disagree. Unique
ordinary-symbol JSON retains its established fields. A static producer remains a
source module, never the ordinary Type it exports. Diagnostics serialize both
flat related notes and persistent producer/caller context notes in renderer order.

Source completion/signature and source rename remain available without a unique
concrete runtime owner. Raw operation/binding metadata requires one concrete owner;
multi-origin singular definitions and affected single-origin rename plans are
refused. See [editor analysis](editor-analysis.md) and
[rename preservation](editor-rename.md) for precise ownership and transactions.
