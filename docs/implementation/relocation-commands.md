# Moving a project directory from the command line

T153 connects the [complete relocation planner](relocation-preview.md),
[reference/graph validation](relocation-references.md),
[staging](relocation-staging.md), [transaction](relocation-transaction.md) and
[independent recovery](relocation-recovery.md) to explicit utility commands under
[TOOL-01 through TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-01).
No additional source-language syntax or alternate resolver is introduced.

```sh
./ama --relocate-preview project project/old --destination project/new
./ama --relocate-apply project project/old --destination project/new --recovery recovery-storage
./ama --relocate-recover recovery-storage/.ama-relocation-ABCDEF
```

The first positional path is an ordinary configured source root. `S` is the second
positional path; `D`, supplied by `--destination`, is **S's exact final location**.
It is not an implicit containing folder. Destination-parent/root topology,
permissions, aliases and collisions are checked by the existing planner. The
recovery storage directory must already exist and remain outside the selected
source tree and invalidated output subtrees.

Preview and apply are separate operations. Preview is read-only and produces no
private staging directories or journal. It resolves all active sources, prepares
exact edits and independently checks the proposed final graph before reporting
readiness. Apply computes a fresh complete plan, acquires exclusive leases,
revalidates inputs and performs the transaction. It does not accept a previously
printed preview as authority to overwrite subsequently changed files.

## Explicit roots and settings

Both planning commands accept:

| Option | Meaning |
|---|---|
| `--root DIR` | Another ordinary source/import root, preserving argument order. |
| `--standard-root DIR` | An explicitly selected standard-library root for double-dot paths. |
| `--exclude PATH` | The driver's shared explicit root-relative exclusion; no output folder is inferred. |
| `--destination-root DIR` | A final ordinary root covering a destination outside existing/carried roots. |
| `--destination-standard-root DIR` | The corresponding final standard-library root. |
| `--entry MODULE` | An optional original logical entry identity; the result returns its mapped identity. |
| `--configuration FILE` | One explicitly supplied supported Ama library configuration; repeat for several files. |
| `--invalidate PATH` | One explicitly selected generated output; repeat for several files/directories. |

Apply additionally requires exactly one `--recovery DIR`. Destination-root flags
are mutually exclusive. Scalar authority occurs once; empty/duplicate values and
inapplicable options reject. Recovery accepts only its single exact journal
directory; roots and settings come from that journal, not fresh guessed command
arguments. No executable entry is required just to relocate a source domain.

The planner returns updated root paths/kinds/exclusions. Carried roots move with
S; external roots remain external. These externally supplied settings are
returned to the client rather than written to an invented project configuration
format. Explicit Ama-library export/module fields and foreign-file settings use
the existing supported configuration parser and original-base resolution rules.
Unrecognized configuration formats, native C strings, comments, literals and
opaque assets are not searched for path-looking text.

Files newly covered by an explicitly added destination root join original
analysis. An original name conflict rejects even when the final mapping would
remove that conflict. For example, a newly covered `.old.a` cannot silently
replace the original `.old.a` just because that original would move to `.new.a`.
The reference layer reports this as `invalid-reference` with the more specific
`ambiguous-module` diagnostic and both source locations. The CLI preserves the
core's stage/category distinction rather than classifying errors by prose.

## Exact result and outcome

All three commands emit `ama.relocation.v1` JSON using the
[lossless source-tool transport](source-tool-commands.md#lossless-response-values).
Physical names have exact hexadecimal bytes and an optional readable UTF-8 view;
integer metadata is decimal text rather than a potentially rounded JSON number.

A completed report includes source/final directory, optional mapped entry,
before/after root settings, every module's physical/logical mapping, file roles,
preserved drafts and the rebuild requirement. File rows distinguish original
existence, source/configuration rewrites, opaque entries and invalidated outputs.
Preview/apply also expose exact original source path-prefix intervals and their
replacement identities. Recovery uses the persisted report and returns no newly
invented edit list; it never applies a source rewrite a second time.

The envelope distinguishes:

- `success`: whether the requested core operation completed successfully.
- `status`: the exact stable category corresponding to `RelocationStatus`.
- `state`: readiness, rejection, committed publication, completed rollback or
  incomplete work. A rejected preview has no pending source mutation.
- `outcome_known`: whether the operation has authority to describe commit versus
  rollback. `committed` is **null** when authority is unavailable or a provisional
  commit decision remains unresolved; it is not falsely reported as uncommitted.
- `finished`: the requested outcome/cleanup is acknowledged. A completed rollback
  can be finished while an apply request remains unsuccessful.
- `journal`: the retained exact recovery directory, if one exists.
- `error` and `cleanup_error`: both retained causes and all original/related
  diagnostic locations, rendered before their owners expire.

A false/unknown outcome must not be treated as a safe instruction to delete a
journal. Recovery data and affected paths remain accessible. Busy or malformed
journal refusal cannot certify that an earlier operation rolled back.

## Output acknowledgement is not a commit decision

Application/recovery completes its filesystem work before the driver writes its
result. Process success also requires complete response writing and flushing. If
a pipe or output stream fails after commit, the process returns nonzero but the
move remains committed. The error stream explicitly says that commit occurred
and gives the retained receipt directory. It does not claim rollback or retry
source edits.

Run `--relocate-recover` on that receipt to obtain the durable result again.
Terminal receipts remain in the requested recovery storage. Repeated recovery
acknowledges the historical outcome without modifying later public edits.
Uncommitted journals restore originals; committed journals complete owned cleanup.
Uncertain acknowledgement/retraction, foreign public state, changed replacement
bytes or unacknowledged nonempty private storage retain data and report failure.

The supported filesystem/exclusive-writer boundaries are the existing Linux
lease/no-replace-rename contract. Clients must keep required project/recovery
locations accessible until pending recovery finishes and coordinate cooperating
writers through the same exclusive operation. There is no timeout-based lock
stealing, directory-name ownership guess or fallback merge/overwrite behavior.

## Ownership and correctness

The command owns preview, original references, staged bytes and an independent
report, then destroys them in dependency order. The transaction borrowing the
staging plan expires first. Recovery owns its own decoded report/source metadata;
it needs no vanished compiler/executor owner. All error spans and related arrays
remain live through structured output and human diagnostic rendering.

Argument arrays are bounded by argv count; extra configuration/invalidation arrays
are allocated only for planning commands. Each repeated option consumes distinct
arguments. Root kinds and optional entry identities are supplied explicitly.
Report loops scan finite owned records; encoders preserve byte/string boundaries.
No report is consumed before readiness. The driver invokes exactly the existing
validated preparation/application/recovery APIs and performs no independent source
rewrites, constructors or cleanup on destruction. It adds linear report handling
to the existing component costs.

## Acceptance

- `make -C tests/unit/driver`: exact operation recognition, standalone recovery,
  explicit destination authority, root kinds/exclusions, option allocation
  prefixes, complete read-only plans, all measured command preparation failures,
  unknown recovery outcomes and an actual refused output stream after commit.
  The latter verifies the moved source and recoverable committed receipt despite
  process failure.
- `make -C tests/integration/tools/relocation`: actual preview/apply/repeated-recover
  processes use a complete Unicode/multiword-path project, outside unreachable
  importer, explicit library settings and inside/outside generated outputs. Opaque
  binary/native/draft bytes survive, the final project passes ordinary checking,
  and initial collisions in newly introduced roots reject without moving S.
- The T110–T114 component suites retain the exact graph, changed-input, filesystem,
  I/O/allocation and process-interruption acceptance contracts. The CLI delegates
  those operations rather than introducing a second transaction engine.

Per-file evidence and final verification are recorded in
[ready.json](documentation/ready.json) and [TODO.md](../../TODO.md). T115 remains the
combined editor/debugger/project-build acceptance workflow.
