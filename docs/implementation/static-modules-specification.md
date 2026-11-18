# Separate static modules: specification change

## Task identity and scope

- Task: [T202](../../DONE.md#t202).
- Status: complete, 2026-11-18; input revision `43afdd9`.
- Authorization: the user requests transferring the discussed static-module model
  into the language specification and EBNF, with detailed motivation and a commit.
- Existing partial work: T182 source, tests and implementation notes remain
  unfinished and are outside this specification commit. The initial record diff
  is retained at `/tmp/ama-t202-existing-records.patch` for staging review.
- Authority: [templates](../../spec/language/templates.md),
  [static modules](../../spec/language/static-results.md), and
  [grammar](../../spec/grammar.md); retain stable rule anchors for source links.

## Contract and planned steps

| Step | Purpose and deliverable | Verification | State |
|---|---|---|---|
| 1 | Record the user's exact redesign and any remaining source-form question | Compare the final conversation decisions and canonical requirements | Done; AMB-014 pair explicitly approved |
| 2 | Separate executable Type placeholders from named static graphs; explain examples and motivation | Cross-review the two language chapters and grammar | Done |
| 3 | Reconcile declarations, source naming, compiler/native boundaries, conformance and navigation | Search obsolete active contracts and resolve all impacted links | Done |
| 4 | Record AMB-013 resolution and implementation alignment prerequisites | Preserve T182 evidence without claiming code acceptance | Done; T203 is the next implementation prerequisite |
| 5 | Run document checks, inspect explicit staged paths, archive T202 and commit | Actual link/anchor/grammar/documentation results and staged diff | Done; final maintained-input regeneration and staging gate before commit |

## Agreed design boundary

Static modules start with `@...` and contain compile-time structural connections.
Free input names are collected after all receiver definitions, every receiver is
public, and nested applications may return several related types or partial
constructors. Incoming constructors retain their actual nominal origin, captures
and residual signature; requirements are inferred from all named uses rather
than restricting them to an exact predeclared signature. Runtime code uses simple
Type placeholders or concrete types. A static export is selected by its exact
name on one indented line after the static module path; no `::` or ordinal export
selector is introduced. Static export bindings are independent of the consuming
runtime context. Distinct cycle/occurs/eager-ownership checks remain mandatory.
Proven-decreasing static recursion stays deferred with dependent types.

The user explicitly approves a two-line own-value definition: `...dictionary
type`, followed by `.dictionary` and its indented named inputs. Multiple pairs
define multiple public outputs. Actual static producers still require named
output reads. Neither trailing own-value capture arrows nor static positions
remain. AMB-014 records this decision and its source. Relative module paths are
explicitly deferred by the latest user instruction and are outside this change.

Binder classification retains a distinction between definition and equality:
explicit pair names are collected module-wide; a projection into such a name is
an equality check. Otherwise the first lexical projection receiver defines the
name and subsequent receivers constrain it. Whole-module visibility does not
make changing the defining occurrence an independent equation permutation.

## Verification and continuation

No compiler source or maintained tests are changed by this task. Document
validation is not reported as implementation support. Three bounded reviews
covered the main chapters, EBNF and collateral requirements; the final independent
review found the ordinary/static, residual/unknown, definition/equality and
nominal/eager-ownership boundaries consistent.

The document checker parses real Markdown links outside code fences, checks their
destinations and anchors, compares unique canonical rule definitions with every
requirement-index destination, and checks EBNF definition/reference closure,
operator alignment and balanced groups after removing quoted/special sequences.
The expected 214-rule set is unchanged from input HEAD. Intentional negative
controls remove a referenced production, shift a definition operator and leave
an EBNF group open; each is rejected. This checks document structure, not compiler
acceptance of source examples. Positive/negative semantic examples were reviewed
against the canonical contracts and grammar.

Commands and observations on 2026-11-18:

- `python3 /tmp/ama-t202-check.py .`: initial run found three links to the
  renamed Stage 11 heading. Restoring its stable explicit anchor fixed all three;
  rerun passed: 28 documents, 1107 links, 214 rules, 11 EBNF blocks and 108
  productions. Removing an unfinished-T182-only link gives 1106 checked links.
- `make docs-complete` in the original working tree passed: 2743 ready inputs,
  zero debts. This scope includes the pre-existing unfinished T182 inputs and
  is not acceptance of the code or the prospective commit alone.
- `git diff --check` passed. The old mixed/positional/interface-preview contracts
  were searched and removed from active requirements; deferred relative paths
  were not added. All 214 stable requirement IDs and index destinations remain.
- A separate snapshot at `/tmp/ama-t202-delivery-og7144ns` was assembled from
  `git archive 43afdd9` plus this task's explicit specification/workflow paths,
  excluding unfinished T182 code, tests, private notes and coverage changes.
  The same document checker passed there (1106 links). `make docs`
  there completed with exit 0: 2690 ready inputs, zero debts, 2032 maintained
  file pages, 7914 documented interfaces and 428 checked canonical/referenced
  destinations. Tools: Doxygen 1.11.0, Python 3.14.4, Markdown parser 3.0.0.

Final delivery gate: after archiving T202 and finishing these records, copy
only the task paths into the same isolated snapshot, rerun the document checks
and `make docs`, and require success before committing. Generated outputs remain
unversioned. The staged diff must contain only specification/workflow records;
all source/test/private-registry changes remain outside this commit.
The unfinished T182 continuation note stays with its working-tree files; T203
must inspect that actual state and plan alignment before implementation resumes.
