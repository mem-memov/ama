# Native publication transaction

T139 separates filesystem publication from scaffold construction. The contract
preserves the existing behavior while making each phase independently testable.
[CABI-06](../../spec/implementation/native-interface.md#cabi-06) requires explicit
native generation that preserves handwritten inputs; the
[architecture rules](../../spec/implementation/compiler.md#arch-03) require narrow
components. This implementation boundary also serves project assembly publication.
It does not implement compiled-library exports or relocation.

## Domain and phase ownership

A ready scaffold owns a finite ordered artifact list with complete immutable
bytes, unique destinations and a valid output model. Fresh transient flags are
false and stage/backup paths are empty. Its owning model keeps destination/byte storage alive; the publication arena
owns temporary paths, and frontend/input/options lifetimes span publication. One caller owns the
scaffold and follows the exclusive output-lock protocol. Filesystem operations
may fail; uncooperative external modification, process crashes and power loss are
outside the multi-file rollback guarantee. The selected POSIX filesystem must
support the required hard links and ordinary rename semantics.

| Phase | Responsibility | Owned resources after success/failure |
|---|---|---|
| Directory | Check/create ordinary prefixes; reject non-directory components | Arena path; created directories intentionally remain after failure |
| Lock | Create `.ama-native.lock` exclusively, close its descriptor once | Only a successfully created lock belongs to this transaction; even a close error requires releasing that lock file |
| Manifest/preflight | Check scope/digests and all exact destinations before staging | In-memory records and existence flags; no destination bytes changed |
| Staging | Create exclusive sibling temporaries, write/close bytes, set executable mode, prepare hard-link backups | Per-artifact stage/backup paths for success and partial failure cleanup |
| Publication | Replace existing files by rename; link new files exclusively | Mark each artifact published only after its filesystem operation succeeds |
| Cleanup | Remove scratch files on success; restore the changed prefix on failure | Ordinary rollback restores old files/removes new files; failed restore retains its backup and reports recovery needs |
| Unlock | Remove only this transaction's acquired lock | A release error is reported independently of already committed artifact state |

The public entry in `output/publish.c` composes the phases. Private declarations
are in `output/publication/publication.h`; directory, lock and preflight have
separate children, staging owns temporary/stream/backup preparation, and commit
owns publication/recovery. Artifact records live in a frontend-free header. Filesystem children receive
only an arena/error pair plus explicit paths, artifacts and a synchronous ownership
policy. The model/manifest ownership adapter is confined to the parent. Children
include no frontend or scaffold-model header. Each extracted file has a purpose
and canonical rule links; Doxygen generates file/interface descriptions.

## Publication and exact completion point

A scaffold must be ready and not previously attempted. A valid attempt is
consumed before directory work, including failed attempts; a premature nonready
call fails before consuming it and leaves an error-bearing owner to destroy.
The caller must not reset an error or retry the same completed attempt.

Directory work precedes locking. After acquiring the lock, build the ownership
manifest and preflight all destinations. Stage **every** artifact and backup
before publishing the first. Artifacts publish in model order; the manifest is
appended last. Thus preparation failure changes no destination bytes, while
publication failure can have a changed prefix that must be restored.

Completion is reached after the last artifact has been published and ordinary
scratch cleanup has run; `scaffold.published` then becomes true. Unlocking follows
and may still fail: the public function can return false while the artifact set
is already committed. That outcome must not be described as a rolled-back
transaction or retried on the same owner. Temporary-removal errors are best-effort
and do not rewrite the primary diagnostic, matching the retained behavior.

The filesystem has no atomic multi-file commit here. Cooperating writers are
serialized; outside readers are not promised a single instantaneous transition.
A failed restore records an additional cause and preserves the affected backup
instead of deleting the last old version. Other cleanup continues. Manual
inspection/recovery is required; success is never reported for a partial rollback.
Lock age never authorizes stealing or deleting a lock owned by another attempt.

## Resource and prefix invariants

Every created staging descriptor has one stream/descriptor owner and one close
attempt. Stream-open failure closes the descriptor directly; subsequent paths
close the stream. Backup creation closes its temporary descriptor before replacing
that temporary name with the old destination's hard link. Paths are recorded before
fallible preparation so cleanup can find any completed prefix. Descriptor close
errors are reported without unsafe retry; host descriptor semantics remain the
same assumption as before extraction.

Before publication, each successfully staged artifact has complete bytes and,
when replacing a destination, an old-version backup. After k publications,
exactly the first k artifacts have `published` set; later destinations are intact.
On a subsequent failure, cleanup visits this finite set and restores existing
paths or unlinks new paths. Each successful restoration clears its published
flag. If all restorations succeed, the public destinations equal their entry
state. If any fail, the remaining flag and backup identify incomplete recovery.
Staging/backup removal cannot discard a failed restoration's retained backup.

Extraction preserves the original branch predicates, syscall order, diagnostics
and arena ownership. Lock state makes its cleanup obligation explicit, including
close failure, rather than hiding it in local control flow. The transaction loops
and per-artifact prefix invariant compose those phase contracts for any finite
artifact list, not only the fixture's number of outputs.

For F artifacts, B total bytes and P total path bytes, staging costs O(B + P),
publication/cleanup use O(F) filesystem operations, and additional path state is
O(P). Manifest parsing/digest validation has its separately documented cost.
Termination assumes the host operations return; no timeout, stale-lock age or
filesystem heuristic changes the operation's meaning.

## Tests and limitations

`make -C tests/unit/native/scaffold/filesystem` links only the filesystem children
and source allocation/bytes. Its five cases construct no frontend, project,
semantic graph or scaffold model: they check binary bytes/executable mode,
first-error ownership policy, restoration, preparation/allocation errors and
exclusive lock ownership.

`make -C tests/unit/native/scaffold/publication` composes the actual frontend,
model and transaction. Five cases cover readiness/reuse guards, exclusive lock
ownership/release error, every observed fresh-publication link failure, every
refresh-backup link and replacement rename failure, and a second rename failure
during rollback. That last case checks both the retained old backup and continued
cleanup of every other temporary.

Successful baseline transactions determine the finite operation counts. Each
one-shot fault ordinal is then exercised on an independent fresh fixture, with
real POSIX operations for every other call. Tests compare actual destination
bytes, including the ownership manifest, and enumerate directories to reject
leftover stage/backup/lock files after ordinary cleanup. The rollback-failure case
checks the actual retained backup. No timing race, sleep or mocked successful
filesystem outcome supplies evidence.

The test-only GNU ld `--wrap` facility redirects `link`/`rename`; it adds no
production hook. Required wrapper flags append even when the caller supplies
other linker flags. Existing ownership tests retain manifest, edited/unknown
outputs, symlink, retained-lock and every allocation-failure-prefix checks. Run
`make -C tests/unit/native/scaffold` for the real declarations/ownership/publication
parent (including the standalone filesystem child) and `make -j4 test` for CLI/native/project composition. No process-crash,
power-loss or uncooperative-writer guarantee is inferred from these tests.

T139 acceptance passed both strict independent leaves (including caller-supplied
linker flags), the real scaffold parent and complete `make -j4 test` with 62
registered leaves. Doxygen checked 462 maintained code/include pages and all
canonical rule destinations; both extracted headers compiled independently.
The registry records 52 ready inputs and 566 explicit remaining debts. The
filesystem child also passed ASan/UBSan with `detect_leaks=0`; no LeakSanitizer
certification is claimed. The canonical specification was not changed.
