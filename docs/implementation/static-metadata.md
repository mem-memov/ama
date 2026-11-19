# Actual static metadata and use requirements

## T208: contract and current step

T208 follows T207's complete local namespace and T213's checked source roles.
Its canonical rules are [TPL-05](../../spec/language/templates.md#tpl-05),
[TPL-07](../../spec/language/templates.md#tpl-07),
[TPL-08](../../spec/language/templates.md#tpl-08) and
[TPL-12](../../spec/language/static-results.md#tpl-12).

Actual metadata and requirements have different meanings. An actual ordinary
constructor retains its original definition, every fixed capture and every
unfilled formal. A use demands operations on that value; it cannot replace the
actual's full residual signature with the smaller set of names it mentions.
An unknown forwarded value remains unknown until supplied, without defaulting
to Type. A static graph is a producer, never an ordinary nominal value.

Completed cohesive steps:

1. Separate ordinary constructor metadata from static producer records and
   examine all complete graph uses before accepting supplied metadata. Preserve
   source scope, exact original names and every actual residual.
2. Add a finite, read-only requirements check over recorded named applications.
   Known actuals must support each named member and operand category; unresolved
   facts remain pending. Additional actual residuals remain legal. Source-order
   permutations and different valid actuals must have the same result.
3. Remove the unused public expected-signature injection interface and its
   sole-purpose fixtures. Rebuild affected tests from current module sources;
   retain useful algebraic equality/identity coverage with current contracts.
4. Verify actual inventory/term/resolver compositions, category/name conflicts,
   forwarded-only values, captures, independent residual maps and allocation
   failure. Update per-file registrations and Doxygen links, archive and commit
   only after the affected strict/sanitized and documentation gates pass.

Component acceptance passes; final rendered documentation and staged-byte
equality remain the commit gates. The obsolete expected-signature injection API, its owner fields,
three source files and sole-purpose test leaf are removed. Actual constructor
signatures are prepared only for ordinary definitions. Graph producers and named
outputs no longer acquire a guessed Type category. Ordinary nominal construction
refuses graph origins, and ordinary captures require Type arguments. T209 owns equation solving/application normalization; T211
owns producer specialization and forwarding equations; T212 owns concrete export
admission. Their remaining work must not be replaced by guessed metadata here.

## Requirement representation and correctness

The complete immutable occurrence inventory is the requirement set. No supplied
actual mutates it, installs a first-actual contract or replaces a residual map.
A query checks every head use of one exact scoped name (including the aliases
below). A named application requires Template, each supplied whole name must
belong to the full actual residual signature, and each known operand category
must match its original slot. Additional actual names are deliberately ignored
by membership checking and remain in the original immutable value.

A bare graph result pair `...alias` / `...source` is an exact local forwarding
equality. Before publication, each symbol starts in a singleton class. For each
such equality, all members of its two current classes receive their earliest
original definition as representative. By induction this is exactly connected
component membership in the finite undirected equality graph: each update adds
one proved equality and preserves all previous ones. No operation assumes an
applied result equals its head or a static producer equals its output. Cycles
and repeated equalities terminate after the finite symbol scan. The canonical
representative never depends on allocation order or future actual values.

Queries conjoin all operations on all names in that class. A forwarded-only
class imposes no fabricated category/signature. A bare operand in the same
class uses the supplied actual category; this detects passing a constructor to
its own Type slot even through an alias. Other unresolved operands remain
pending for T209/T211 equations. A known conflict dominates an unrelated pending
operation. The public result certifies satisfaction of currently represented
requirements only; it does not certify complete actual admission or execution.

For S local symbols, preparing direct representatives takes O(S squared) time
and constant extra storage. For U occurrences, B considered named bindings and
K maximum actual slots, a conservative per-query bound is O(U + B(U + K)):
source metadata lookup and exact whole-name membership are linear. Queries
allocate nothing and modify no source, actual, capture or residual. This avoids
per-actual retention and makes admission order irrelevant. Existing inventory,
term and planner owners retain normal teardown on all allocation failures.

Tests use current runtime name/type pairs for ordinary formals and exclusive
`@...` graphs for Type/Template operations. Two- and three-residual actuals,
fixed unused captures, staged forward normalization, wrong categories, missing
names, pending operands, forward-only values, alias order/cycles and source-kind
separation are composed through real inventory and nominal/planner interfaces.
The retained identity/nominal/failure fixtures are migrated without discarding
their useful scope, equality, ownership and failure assertions. Source-request
and producer equation fixtures retain their later T209/T211 owners; this is not
a claim that the entire historical static test parent is accepted yet.

## T208 acceptance record (2026-11-19)

Nine strict C11/Werror groups pass on the task-only tree:
`usage`, `scope`, `explicit`, `classify`, `composition`, `signature`,
`terms/identity`, `terms/nominal`, `terms/failures`, all under
`tests/unit/semantic/static/`. These include bare-forwarding classes, alias
cycles, current-source nominal fixtures and real staged forward application.
Commands use `make -C <leaf> -j2` with
`C_COMPILER_FLAGS=-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L`.

The same nine groups pass O1 ASan/UBSan with
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`,
`LINKER_FLAGS=-fsanitize=address,undefined -no-pie`, and
`ASAN_OPTIONS=detect_leaks=0`. This retains the established container restriction;
counted teardown and the migrated term failure/overflow tests independently
check ownership. The requirements query and forwarding-class construction
allocate no memory; query checks explicitly preserve the allocation count.

The isolated root `make -j4` and strict
`make -C tests/unit/compiler/native-admission -j2` pass. All job exit statuses
were collected. Acceptance input is HEAD plus T208 in
`/tmp/ama-t208-check-a6kquvp2`; unrelated unfinished T182 changes are excluded.
Complete per-file/header checks pass with 2704 ready inputs and zero debts.
Final Doxygen rendering, rule destinations, source fingerprints, queue/link
review and exact staged-byte comparison are commit gates; the commit records
their final output. No full static-parent or executable-template pass is claimed.

No active test/build jobs remain. Later source-request, producer expansion and
recovery acceptance remains with T209–T212 and the existing roadmap. The removed
expected-signature injection API is not retained as a compatibility alternative.
