# Prepared source compiler bridge

## T273 contract

T273 starts from accepted T272 commit `44e813c5` and a clean worktree. It is the
next explicit prerequisite extracted from T224's existing original-analysis
step. T224 remains responsible for physical selected-root capture, test-entry
probes, source selection, mapping, rewrites, transactions and public operations.
The [T272 source-operation record](source-operations.md) defines the existing
`ama.source-snapshot.v1` input and `ama.source-domain.v1` output boundaries.
The canonical authority is [PROJECT-01](../../spec/implementation/project-tools.md#project-01)
and [TOOL-02/04](../../spec/implementation/tools-and-conformance.md#tool-02).
No specification change is part of T273.

## Ordered work

1. Extract the captured string/path/text ownership and coherence component shared
   by the existing input reader and the new response reader. Keep syntax scratch,
   retained rows and source byte lifetimes explicit; remove the replaced private
   input-only implementation rather than retaining a parallel decoder.
2. Decode the exact closed response schema into plain owned versions, sources,
   native capture observations and original definition/edge records. Require
   canonical unique identities, source-backed bounded edge spans, known targets,
   exact categories/booleans and consistent raw bytes. Preserve observation
   absence separately from uncaptured fields. Do not infer executable tests from
   original parameter counts or inspect Ama/C source to reconstruct metadata.
3. Write the existing input schema from an explicit borrowed captured table. Do
   not invent a decoder-valid flag for manually prepared provider data. Keep
   version/source/native authority and integer identities exact. Accept the writer
   independently and through the actual captured frontend/read-only command.
4. Add shell-free process orchestration using the existing project process API.
   Require normal successful exit before consuming output; signals, spawn/wait/
   redirection/stream and compiler failures never produce a usable graph. Share
   a narrow stream-reading primitive with existing raw source I/O where needed.
   Retain raw compiler diagnostics separately from protocol decoding diagnostics.
5. Compare returned source/version/configuration facts with every supplied input;
   native observations must match the corresponding supplied companion capture
   after actual compiler classification. Reject missing/extra/changed records
   before metadata is admitted. Structural JSON validity alone is insufficient.
6. Accept components, real compiler/project process handoffs, invalid responses,
   ownership/byte paths, every allocation and relevant stream/process refusal,
   independent project build, sanitizers, per-file evidence and Doxygen. Archive
   and commit T273 only after its complete bridge contract passes.

## Ownership and progress

All bridge results borrow no live frontend or compiler AST. Original source spans
refer to result-owned captured byte snapshots; names and graph rows have explicit
operation owners. Temporary request/response files or streams belong to the
bridge caller/operation and never alter a project source. The compiler sees only
the supplied finite table. Parsing, exact equality/index scans and finite record
construction provide explicit termination/cost arguments; process completion
remains an observed host operation, without a heuristic timeout or fallback.

No implementation or acceptance is claimed at this initial planning checkpoint.
Next action: introduce the shared coherent capture owner and migrate the existing
input decoder, then implement the response decoder on the same primitive.


## Shared capture step

Introduced `src/source/json/capture/` with explicit retained names/index/source
owners and independent syntax/byte scratch. Exact repeated physical paths reuse
only identical content; binary source/ABI bytes remain opaque until their actual
role is validated. Empty/NUL paths and aliased result/scratch arenas are refused.
Migrated the existing captured-input reader and removed its replaced private
`src/delivery/source/snapshot/text.c` implementation and private helper interfaces.
The input model now owns `SourceJsonCapture` separately from its row arena.

Strict existing captured-data/frontend parent passed after migration (session
59518, `/tmp/ama-t273-shared-capture.log`, exit 0). The new independently runnable
five-case capture leaf passed (`/tmp/ama-t273-capture-unit.log`, exit 0), proving
exact binary/byte-path snapshots, coherent reuse/conflict refusal, result survival
after syntax/scratch/original JSON teardown, explicit nulls, malformed fields,
every allocation refusal and invalid/aliased owners. The neutral schema leaf now
links only its own syntax/schema dependencies; shared capture behavior has its
own direct tests and real existing decoder consumers.

Current checkpoint: T272 is committed as `44e813c5`. T273 has an uncommitted
planned shared-capture migration and test/evidence updates. No jobs are running.
Next: implement the closed response data owner/decoder using this same capture
primitive, then input serialization and shell-free process/result admission.
No T273 completion, project-side compiler integration or public source operation
is claimed yet; its final gates and commit remain pending.

## Response, serialization and comparison steps

Implemented a plain owned closed-schema response reader, independent of live
compiler objects; retained byte paths and bounded original dependency spans use
the shared capture owner. Added the borrowed `DeliverySourceSnapshotView` and
stream serializer without fabricating a decoder validity flag. Moved the shared
wire byte/blob formatter out of report-private storage, removing that replaced
implementation. Exact reply admission compares source/version sets and complete
configuration bytes and requires captured native observations to match the
supplied companions. Singleton lookups reject ambiguous provider records. Equal
cardinality and singleton membership establish finite set equality; scans are
quadratic in rows plus compared byte lengths, without recursive expansion.

Initial ordinary compiler-warning runs passed for the full delivery/source parent
(`/tmp/ama-t273-delivery-current.log`), the independent matching leaf
(`/tmp/ama-t273-matching.log`), writer view migration
(`/tmp/ama-t273-writing-view.log`) and controlled child client
(`/tmp/ama-t273-client.log`). Those invocations used `CFLAGS`, which the leaf
harness does not consume: they are not `-Werror` evidence. Correct strict reruns
use `C_COMPILER_FLAGS`; active logs are `/tmp/ama-t273-{delivery,client,json,compiler}-strict.log`.

Added shell-free project compiler orchestration, owned raw stdout/stderr,
normal-success admission before decoding, and exact response/input comparison.
A controlled child executable tests success, literal argument punctuation,
nonzero 127 versus missing executable, signals, malformed success, mismatched
success and every result allocation failure with zero live counted allocations
at teardown. Extracted borrowed stream reading from source loading, removing the
private duplicate; the direct stream leaf passed with strict `-Werror`
(`/tmp/ama-t273-stream.log`), testing arbitrary binary multi-chunk bytes, EOF,
caller stream ownership, read errors, invalid arguments and each growth failure.

T273 remains active. Next: collect strict jobs, check actual compiler handoff and
independent project build, update per-file coverage, run sanitizers and Doxygen,
then archive and commit only after complete acceptance. T224 public source
operations remain unfinished.

Strict delivery/source parent and neutral JSON parent completed successfully
(`/tmp/ama-t273-delivery-strict.log`, `/tmp/ama-t273-json-strict.log`, exit 0).
Strict controlled-child client completed successfully
(`/tmp/ama-t273-client-strict.log`, exit 0). Root strict build initially failed
because the separate project executable omitted its new plain response reader
link inputs; corrected `tools/project/sources.mk` to include that reader, with no
compiler frontend, dispatcher, parser or semantic state. The exact strict root
rerun `/tmp/ama-t273-compiler-strict-fixed.log` completed with exit 0. Existing
source text load/ownership regression passed with strict flags
(`/tmp/ama-t273-text-regression.log`, exit 0).

Added a real compiler handoff case to the project client after the controlled
process cases; its current run is `/tmp/ama-t273-client-real.log`. The independent
private project build/link-membership regression is `/tmp/ama-t273-project-build.log`.
Both outcomes must be collected before completion. Per-file registry now maps
new response/writer/matcher/client/stream inputs and the shared wire move to
explicit named cases. Documentation generation and sanitizer acceptance remain
pending.

## Isolation correction and remaining final gates

The first real-client strict run failed while decoding a concurrently rewritten
shared generated fixture (`/tmp/ama-t273-client-real.log`, exit 2). This exposed
fixture ownership, not an accepted compiler result. Matching and client leaves
had invoked the response generator inside its sibling directory. Changed the
generator to locate helper code relative to `__file__`, while all output remains
relative to the invoking leaf. Callers now execute it without changing working
directory and load only their own `generated/` files. Client fixtures are immutable
and generated once per explicit run, avoiding needless regeneration at every
allocation fault. The independently owned delivery parent and client now run
concurrently as an isolation regression (`/tmp/ama-t273-{delivery,client}-isolated.log`).

The first client strict job also overlapped a later sanitizer job's output tree;
its outcome is superseded rather than reused. Sanitizer delivery/client jobs
exited 0, but their earlier shared fixture overlap is likewise superseded. Final
sanitizer acceptance must use isolated fixtures and serialize each leaf against
its own earlier runs. Direct capture and stream sanitizer leaves completed with
exit 0 (`/tmp/ama-t273-{capture,stream}-sanitizers.log`).

Private independent project build/link invalidation regression completed with
exit 0 (`/tmp/ama-t273-project-build.log`). Strict existing source module,
package-capture and native ABI integer/integer-arithmetic leaves all completed
with exit 0 (`/tmp/ama-t273-{module,packages,integer,integer-arithmetic}-regression.log`).
First Doxygen generation completed successfully (`/tmp/ama-t273-docs.log`),
including all 3523 registered inputs with zero debt. That is historical evidence;
these later maintained corrections require final regeneration.

## Completed component acceptance

After isolation correction, concurrent strict delivery parent and client passed
with exit 0 (`/tmp/ama-t273-delivery-isolated.log`,
`/tmp/ama-t273-client-isolated.log`). The latter includes the actual compiler
snapshot handoff. Extended exact matching with reordered provider rows, native
header mismatch and version-content mismatch. Added real `RLIMIT_NOFILE`
exhaustion for both missing temporary streams and second-stream failure,
restoring the original process limit before cleanup. Final strict matcher and
six-case client passed (`/tmp/ama-t273-matching-final.log`,
`/tmp/ama-t273-client-final.log`, exit 0).

Final isolated ASan/UBSan delivery parent and client passed with exit 0
(`/tmp/ama-t273-delivery-sanitizers-isolated.log`,
`/tmp/ama-t273-client-sanitizers-isolated.log`). Commands use strict C11 flags,
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`,
`LINKER_FLAGS=-no-pie`, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer remains unavailable under the
previously recorded ptrace restriction: these are not LSan results. Every
allocation-fault test separately checks zero live counted allocations after
teardown. Direct captured owner and stream sanitizer evidence remains valid.

The moved wire formatter's actual report consumer passed strict regression
(`/tmp/ama-t273-report-regression.log`, exit 0). The driver source-domain leaf is
still running in `/tmp/ama-t273-driver-regression.log` (session 38978). No other
test job remains. Fingerprinted 86 affected component inputs in
`/tmp/ama-t273-frozen-inputs.json` before final documentation acceptance; these
inputs must remain unchanged. No runtime/native/test execution or filesystem
source mutation is performed by the compiler bridge. No new public project
command is claimed. Remaining gates: collect driver status, final current
Doxygen/manifest validation, archive the complete task record and commit.

## Accepted T273 and final archive delivery

The driver source-domain strict regression completed with exit 0
(`/tmp/ama-t273-driver-regression.log`). Full current `make docs` passed
(`/tmp/ama-t273-docs-current.log`, session 17998): 3523 ready inputs, no debts,
2673 maintained file pages, 10316 extracted documented interfaces, 495 canonical
and referenced destinations and 8433 total pages. Tools remain Doxygen 1.11.0
(commit `9b424b03c9833626cd435af22a444888fbbb192d`), markdown-it-py 3.0.0,
Python 3.14.4 (GCC 15.2.0). All 86 frozen component input hashes are unchanged.
The complete task record is archived in DONE; T224 keeps every remaining operation
criterion. No conformance fingerprint is refreshed.

The real-client leaf requires the root compiler built with `make ama` before its
explicit command; root `make test` already depends on both executables. Controlled
process cases use the leaf's own executable, no shell-generated command string.

Archive changes are Markdown only. Before committing, run the supported
`python3 tools/documentation/build.py --doxygen doxygen` with log
`/tmp/ama-t273-docs-archived.log`, then require the delivered documentation manifest
to equal the build report and every recorded input fingerprint to match current
bytes. Do not change maintained inputs during that run. Stage explicit task paths,
inspect the staged diff and commit T273; next resume T224's physical captured-root
and independent test-entry integration. No background test remains.

Archived narrative regeneration completed with exit 0 (session 24654,
`/tmp/ama-t273-docs-archived.log`), preserving the accepted file/interface/link
counts. The final archive whitespace cleanup changes Markdown only; final
reference provenance is regenerated into `/tmp/ama-t273-docs-final.log` with the
same supported builder before index validation and commit. No component input
or conformance evidence changes as part of this archival cleanup.
