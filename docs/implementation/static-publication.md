# Complete static producers and immutable publication

[T211](../../DONE.md#t211) adapts publication to the actual named `@...` model.
Authority: [TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
Historical T180/T201 acceptance remains in [DONE](../../DONE.md); its former
ordinary static-section syntax is not an alternative supported interface.

## Inputs, results and ownership

The owner borrows the [guarded result plan](static-projections.md). Destroy it
before that plan, its registry, guards, solver, terms and source inventory.
Its arena owns immutable publication/output/alias descriptors. Attempt-local
closure queues use separate scratch arenas and release them on every exit.
Allocation failure preserves the caller's output pointer; retained partial
records remain owned until teardown. Failure is sticky and preserves the first
borrowed diagnostic and its original source location. No rollback of previously
added solver facts is promised.

`static_publication_resolve` closes one actual source use. An ordinary expression
publishes its Type/Template `value` with no bundle context. A named producer
publishes a bundle with null `value` and its actual producer `context`: a static
module has no own ordinary Type or executable instance.
`static_publication_scope` requests all exports of a registered static-module
root without inventing an AST call. Such bundles have null `use` and `value`.
Ordinary roots are refused by that operation.

A bundle contains every public pair result and first-receipt alias in inventory
source order, including names not selected by the caller. Values are queried in
the proven canonical owner environment; recurrence-linked witnesses must not
read disconnected raw scope variables. Named receipts publish immutable aliases
under the exact supplying lexical `(environment, symbol)` identity. Repeated
source publications and root publications reuse their exact original handles;
equal returned metadata does not merge different source occurrences or runtime
state. Existing receiver equalities cannot rebind a name.

## Separate value facts from producer admission

Every producer input must be concrete before its bundle can publish. A constant
selected output does not determine an unrelated missing input. Returned ordinary
Template residuals remain bound in that returned constructor and do not count as
missing producer inputs. No default, origin search, guessed inverse, runtime
body, native initializer or constant execution supplies missing evidence.

An intermediate source or local definition may supply concrete inference facts
before its lexical owner's complete key is known. Closing that fact does not
implicitly require every unrelated lexical output. Requesting the whole root
explicitly requires its complete input map and all public definitions.
Ordinary Type/body checks remain retained registry obligations for later
admission; structural publication does not itself authorize executable output.

The closure graph has three exact, tagged proof domains:

- A source proof needs READY/closed structural work, its required producer graph
  and its actual expression/definition dependencies.
- A definition proof needs its solved metadata and actual checked RHS. A selected
  alias also retains its defining foreign read. A cache hit follows the original
  checked RHS rather than discarding producer requirements.
- A producer graph proof needs its complete input key, local certificate and
  every pair, first receipt and producer statement, including equality-only uses.

Whole-graph validation cycles may close as finite groups; the queue visits each
registered proof handle once. They do not become value-definition edges. Actual
value reads still pass the separate acyclic definition guard. A known selected
output therefore cannot hide an unselected recursive value.

## Refinement, termination and costs

Each attempt advances the selected source and all currently required graph
witnesses, including witnesses appended during that pass. Only the closure
reachable from the requested source/root must complete; unrelated pending roots
can supply inference facts without becoming false dependencies.

Repeat only if an exact component of the evidence tuple changes: unique equations,
metadata facts, requirements, actual contexts, graph witnesses, closed definition
caches, completed local graphs, or required registry dependencies. Term allocation, application snapshots,
attempt counts, depth limits and timeouts are not progress. Stable unresolved
work fails with its original producer location.

Guarded expansion has finitely many actual witness paths: a repeated origin with
unknown keys suspends before expansion, different keys fail, and equal keys link
to an already required actual ancestor witness. Independent incoming paths may
replay; a globally first cached witness cannot suppress useful independent
inference. The set of simple source-origin paths is finite, but may be exponential
in the source graph. Do not claim a source-linear bound. Within that finite
universe exact facts, retained proofs and completion flags grow monotonically;
every nonterminal attempt adds one, so refinement terminates.

For K reachable proof handles and E dependency edges, the linear-membership
scratch queue costs O(K² + EK) time and O(K) memory, excluding solver/guard work.
For O public outputs, B receipts and L stored aliases/publications, descriptor
lookup costs O(O + B(O + L) + L), excluding canonical value queries. Each bundle
retains O(O + new aliases + 1) additional descriptors. Every size counter is
checked before an increment that could exceed representable storage.

## Acceptance boundary

The `publication/{scopes,closure,values,failures,recursion}` leaves exercise
complete roots, multiple outputs/public aliases, partial ordinary constructors,
receiver inference, noninvertible/unused inputs, unselected failures, immutable
identities, exact handle validation, cold/warm ancestry and independent roots.
Failure injection refuses every measured allocation in explicit, inferred and
root publication; counted teardown checks partial ownership. Per-file cases and
standalone header commands are in [the registry](documentation/ready.json).
T211 commands, limitations and acceptance evidence are retained in DONE.
Constants and ordinary named export selection belong to T212; whole-project
ordinary validation and executable admission remain T184/T185 responsibilities.
