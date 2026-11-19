# Ordinary Types and independent static exports

T212 introduces a shared operation for ordinary field and constant-namespace
Types under [TYPE-01](../../spec/language/declarations.md#type-01),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[CONST-03](../../spec/language/declarations.md#const-03) and
[CONST-06](../../spec/language/declarations.md#const-06).
This is implementation work, with no specification changes or executable
admission claim. Acceptance is recorded in [DONE](../../DONE.md#t212).

## Source identity and ownership

The publication owner retains `StaticOrdinaryType` records keyed by the exact
ordinary incoming context and original root expression. A record either borrows
normal guarded source work or names one actual static producer/public export.
A selector never declares a using-module placeholder. Public outputs exclude
free input parameters even if their complete names match the written selector.
Normal applications and static graphs retain their existing owners; there is no
inline-static-application adapter in ordinary code.

All records use the publication arena and expire before results, registry, guards,
solver, terms and source inventory. Failures are sticky, retain original source
provenance and leave caller outputs untouched. Partial records and inference
facts remain owned until teardown, without a rollback claim.

## Independent supplier closure

Registration creates a consumer-local Type variable. For a selected export it
adds no equality to the supplying output. A consumer's expected Type, literal,
call or desired member therefore cannot fill a supplier input. Resolution obtains
the exact path's independent compilation root and retains a registry validation
dependency, without creating an ordinary instance or constructor edge. It closes
the full static root, including every output and dependency, before selecting the
original named export. That export must be complete ordinary Type metadata;
other outputs may remain returned bound Templates.

Only after successful independent publication is an equality added between the
selected canonical Type and the consumer variable. Existing incompatible consumer
evidence then fails instead of rebinding either side. Repeated requests recheck
current constraints and reuse original source/publication identities. No module
name search, default instance, signature matching or contextual factory inference
is permitted.

Normal fields preserve existing contextual inference. READY from evaluation
concerns structural work: forwarding a bare unknown Type can already be READY
while its value remains open. Evaluation can also return Pending; final namespace resolution requires complete metadata and
retains the actual ordinary source publication. A selected static export cannot
wait for future ordinary receiver constraints to identify its producer. Ordinary
body checks remain separate registry obligations, so naming a known Type does
not prematurely certify an entire mutually referencing definition group.

## Algorithm and tests

Exact registration scans source occurrences and owned records linearly. A query
creates at most one record per exact source/context and an independent producer
root reused by the existing root service. Export lookup scans the finite public
output list. It never follows runtime values or enumerates candidate Types.
Termination and cycle handling come from the guarded structural publication
kernel; no source-language recursion or guessed iteration budget is introduced.
Costs are those finite identity scans plus the published solver/graph contracts.

The independently runnable `publication/ordinary` leaf covers separate field
storage identities, shared supplying publication, known consumer versus open
factory, Type/Template selection, unknown/nonpublic names, retained ordinary
inference, handle/counter refusal and every measured allocation failure. The completed T212 record links composed constant integration, strict/sanitizer
and per-file/header/documentation acceptance. Runtime inference and executable
admission retain their later task owners.
