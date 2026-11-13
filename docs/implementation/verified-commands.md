# Check-only and verified compilation commands

T060 accepts the existing shared frontend and explicit publication pipeline under
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[CABI-06](../../spec/implementation/native-interface.md#cabi-06) and
[STACK-04](../../spec/implementation/compiler.md#stack-04).

## Shared validation, explicit effects

`ama --check ROOT .entry` checks the active transitive project without native C
compilation or output publication. `--status` distinguishes drafts from complete
modules; an active draft cannot satisfy an executable dependency. Inactive drafts
do not poison an unrelated active closure.

`ama --emit ROOT .entry --out DIRECTORY --sdk SDK --compile` uses the same
frontend, then stages complete generated source bytes in private build storage.
The configured C compiler checks declarations and the required backend profile;
exact symbol verification checks native-entry inlining and rejects ordinary Ama
C-entry functions before linking. Only a verified executable becomes a ready
artifact for explicit publication. Failures before publication cannot replace an
existing owned executable or authorize overwriting an unowned file.

Output staging records completed writes before replacement and tracks the
successful replacement prefix. Failure cleanup restores that prefix using the
publication component's explicit ownership information. This is the documented
failure-handling boundary of the command, not a claim of one filesystem-wide
atomic rename or crash transaction across unrelated files.

The driver passes original argument elements to its components and does not
reconstruct shell commands. Native compilation is absent from check-only and
emit-without-compile. A complete checked frontend is a prerequisite to every
later stage; missing modules and active drafts therefore produce the same
frontend diagnostic in check and verified-compile modes.

## Evidence

`make -C tests/integration/check` runs
`it_matches_check_and_verified_compile_diagnostics_without_publishing_failed_projects`:
actual CLI invocations check missing and draft roots in both modes, compare exact
stderr, require empty acknowledgement output and verify no output directory was
created. Existing cases check standalone draft status, check-only effects and
emit-without-compile.

`make -C tests/integration/execution` builds and runs the verified executable,
then requests a native compilation failure against the same destination and runs
the previous executable again. Another case preserves an unowned handwritten
program file. `make -C tests/unit/driver` checks command/option validation and
warning/output failures. `make -C tests/unit/native/scaffold/publication` covers
publication stage failures and prefix rollback. Existing codegen execution tests
exercise mandatory compiler flags and rejected entries before executable
publication; T057/T058 record that verified toolchain evidence.

The changed integration harness has named per-file coverage in the documentation
inventory. This checkpoint introduces no alternate frontend, implicit command,
compatibility mode or new specification requirement.
