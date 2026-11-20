# Final template sprint documentation

## T198 contract and steps

Input: `2e3bbc7`, after the complete accepted clean release T197 and current
conformance review T196. The final delivery must describe the current source,
archived tasks and explicit limitations, with no obsolete baseline-only claims.
No specification change is authorized or needed.

Rules: [ARCH-06](../../../spec/implementation/compiler.md#arch-06) and
[documentation delivery](../../documentation.md).

1. Reconcile the root introduction, current conformance guide and component
   navigation. Preserve explicitly historical task/audit findings and useful
   rejected-probe evidence; remove obsolete current-status claims.
2. Verify unique task identities, no unfinished task archived as complete and
   zero per-file debt. Review fingerprints affected by documentation changes;
   unchanged source/test/specification hashes must remain unchanged.
3. Run conformance, then serial `make docs-complete`, `make docs` and
   `make docs-reproducible`. Collect actual results, file/interface/destination
   counts, tool versions and complete generated manifest/source stability.
4. Archive accepted T198, close the active queue and regenerate/recheck for those
   exact final archival inputs. Compare all staged/rendered hashes before the
   final commit. Generated file/interface pages remain Doxygen output, never
   handwritten maintained replacements.

## Initial findings

The root README still describes only the old 200/108 baseline. The current
conformance guide has an obsolete final paragraph denying template support,
despite its current 214/150/56 review. Component navigation still describes
static adaptation as upcoming. These are stale current summaries, not an
unsettled language requirement. T197's new release guide already records the
successful complete current release and normal/instrumented public consumers.

Current step: reconcile these guides and review the affected argument digest.

## Reconciled inputs and reviewed authority

The three stale current summaries are corrected; explicitly historical audits,
subheadings and rejected-probe task records remain intact. Actual task
declarations are the checkbox titles: **218** unique IDs, T001–T218, with no
missing/reused ID and no unfinished task declaration in DONE. Historical
subheadings for T178/T201 are evidence sections inside their one task record,
not duplicate tasks; the initial overly broad heading scan was corrected before
making any archival edit. The next unused task ID is T219.

Production, tests, registry and all 24 specification fingerprints remain exactly
those accepted by T196/T197. Only the reviewed argument digests of the current
conformance guide and component navigation change; both are now refreshed. An
initial one-document expectation correctly refused the second changed contract,
so no unreviewed digest was silently accepted. No checker, code or spec rule was
changed. Current gate session 20699 writes `/tmp/ama-t198-conformance.log`.

Next: collect that gate and run the three required documentation commands
serially against unchanged maintained inputs, with separate result receipts.

## Serial documentation checkpoint

Current conformance passes (`/tmp/ama-t198-conformance.log`): 214 requirements,
150 acceptance rows, 56 groups. The next controller runs, in order:
`make docs-complete`, `make docs`, `make docs-reproducible`. Separate logs are
`/tmp/ama-t198-initial-{complete,docs,reproducible}.log`; current/final receipts
are `/tmp/ama-t198-initial-jobs.json`, with stable-input hashes in
`/tmp/ama-t198-initial-inputs.json` and the tool session ID in
`/tmp/ama-t198-initial-session.json`. No maintained input changes while these
commands run. Next action: collect every result, then archive T198/close the
queue and repeat final serial gates for those exact delivery inputs.

## Accepted initial delivery and final archival checkpoint

All three serial commands pass with all **3285** maintained inputs unchanged:
`/tmp/ama-t198-initial-{complete,docs,reproducible}.log` and
`/tmp/ama-t198-initial-jobs.json`. Actual Doxygen 1.11.0 delivers
2297 maintained file pages, 8908 documented interfaces and 453/453 checked
canonical destinations. Reproducibility compares complete generated membership
and bytes, not just these counts. Python 3.14.4 and markdown-it-py 3.0.0 are
recorded in the manifest. No specification or maintained code changed.

T198 is now archived; TODO keeps future-work instructions, the two existing
external navigation targets and an explicit empty active queue. All T001–T218
records remain in DONE, and T219 is the next unused ID. The final source state
will be checked again by conformance and, serially, `make docs-complete`,
`make docs`, `make docs-reproducible`. Final logs/receipts use the same
`/tmp/ama-t198-final-*` naming and the final tool session is recorded in
`/tmp/ama-t198-final-session.json`.

Next action: collect final gates without changing these maintained inputs;
verify the final manifest against working and staged bytes, inspect explicit
staged paths and commit T198. Generated pages remain unversioned under
`docs/technical/`. The accepted T197 whole clean release is unchanged; no
statement-coverage percentage, unsupported platform, optional optimization,
LeakSanitizer or ThreadSanitizer success is invented.

## Final delivery accepted

Final conformance and all three final serial documentation commands pass.
`/tmp/ama-t198-final-jobs.json` records successful complete/docs/reproducible
results with all 3285 archived inputs unchanged. The generated reference verifies
2297 maintained file pages, 8908 documented interfaces and 453/453 canonical
destinations; independent output membership and bytes are identical. The final
TODO has no active task, all 218 unique completed task declarations are in DONE,
and per-file debt remains zero.

This final result paragraph is the only edit after that accepted sequence.
`make docs-reproducible` is run once more for its exact bytes, with unchanged
source authority checked in `/tmp/ama-t198-delivery-inputs.json` and
`/tmp/ama-t198-delivery-result.json`; log:
`/tmp/ama-t198-delivery-reproducible.log`. Its successful source-stable output
and complete working/staged manifest agreement are mandatory commit conditions.
No additional implementation or specification work remains in the sprint.
