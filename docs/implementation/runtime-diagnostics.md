# Refusal and technical-error diagnostics

T059 composes the generated operation/source table, runtime call chain and
cleanup reporting under [DEBUG-02](../../spec/implementation/diagnostics.md#debug-02),
[ERROR-03](../../spec/implementation/diagnostics.md#error-03) and
[CALL-04](../../spec/language/calls-and-results.md#call-04).

## First cause before cleanup

The [trace reporter](../../src/runtime/program/diagnostics/trace.c) validates a
status before reading optional message bytes. It reports the first technical
failure while the current instance and explicit caller links still exist.
Each operation ID selects an exact generated path, byte column, role and nominal
type. Before a root exists, construction/initialization operation IDs locate the
failure without fabricating an executing instance. Later cleanup errors are
secondary; the reporter does not rebuild an already destroyed call chain.

Reporting streams existing immutable data synchronously and does not allocate
runtime storage. It neither changes a caller's last-refusal snapshot nor treats
that snapshot's nonzero code as a new technical error. Busy/stale operation
refusals remain associated with the initiating caller and their own alternative;
technical construction or native errors terminate execution and preserve their
primary cause through cleanup.

The walk follows validated immediate caller links established by call admission.
At each step it uses the exact typed pool lookup before reading metadata. It
never reconstructs a source name from a C symbol or guesses an operation from a
nearby source span. The first-error flag separates one intact trace from later
secondary messages. Host cleanup retains the initiating status even when one or
more destructors fail.

## Evidence

`make -C tests/unit/codegen/execution` runs
`it_preserves_primary_failure_when_native_cleanup_also_fails`. A nested native
body fails and its destructor also fails: the compiled host observes the original
code/message, source locations cover the inner/outer/root callers, the destructor
message is secondary, repeated disposal preserves the primary status, and pools
and borrows are empty. A second case fails a literal constructor before root
execution while an already constructed native sibling fails destruction.

Existing compiled cases cover Unicode locations, pre-root shared construction,
active nested chains, busy and absent target refusals, caller-owned status
readers, and corrupt return selectors. `make -C tests/unit/runtime/contexts`
runs `it_streams_checked_diagnostics_without_runtime_allocation`, which disables
runtime allocation, reports an intact chain and secondary error, checks ordering
and keeps the caller refusal/link unchanged. The same context suite checks stale
and foreign handles. `make -C tests/unit/runtime/host-lifecycle` runs
`it_reports_the_active_execution_chain_before_releasing_all_context_resources`:
real acquire/prepare/connect transitions lead to one allocation-free failure
report before complete context cleanup. These complement the compiled source-map
handoff rather than replacing it.

The new diagnostic test support and changed fixture/harness have named per-file
coverage in the documentation inventory. No new error category, fallback source
location or specification change is introduced at this checkpoint.
