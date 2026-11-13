# Frontend, driver and project emission contract

T150 documents retained implementation boundaries. Doxygen generates the
file/function reference through `make docs`; this document supplies the shared
ownership and correctness argument. Canonical requirements remain in
[spec/](../../spec/README.md). The [audit](audit/README.md) and
[TODO](../../TODO.md) identify unsupported source forms and future boundaries.

## Driver and frontend lifetime

The executable `src/main.c` only forwards process arguments and standard streams
to the driver. Exact command recognition precedes option validation; the input
domain is a nonnegative count and that many valid NUL-terminated arguments.
Count zero permits a null vector. Standalone help requires exactly two arguments.
Unknown requests fail; no abbreviation, typo correction or inferred command is
used. Options own arrays and borrow argument strings during one command. Streams
are borrowed, may alias, and are never closed by the driver. A success report must
both write and flush successfully. A partial output cannot be rolled back.

A frontend owns project/source storage, proposed authoring ABI bytes, semantics
and resolved plans. It is initialized, attempted once, then destroyed. Stage
initialization flags are set before fallible work, so every prefix can be safely
destroyed. Plans borrow semantics; semantics borrow parsed source. Destruction
therefore reverses that order, keeping source-backed diagnostics valid until the
last consumer finishes. False preserves the stage diagnostic; true requires the
complete semantic and planning sequence. Check-only performs no C compilation,
native execution or artifact publication.

Syntax-only status inspection owns a separate catalog and AST and parses just the
selected module. A draft is a valid inspection result, never executable validation.
Native authoring also remains explicit: a proposed ABI source lives in a frontend
source owner through semantic destruction. It validates the selected native root
against proposed/existing binding records, without pretending unwritten native
hooks are a working implementation. Existing companion replacement follows the
explicit create/overwrite policy.

These lifetime and success laws compose with the
[driver contract](contracts/driver.md), [checking](checking.md),
[project closure](project-dependencies.md), [semantic analysis](semantic-analysis.md)
and [resolved plans](typed-plans.md).

## Model correspondence and exact output

Emitters borrow a valid nonauthoring frontend, scaffold model and supported UTF-8
configuration. The source/descriptor lifetime covers complete emission and any
native verification. Partial output belongs only to an unpublished staged artifact;
artifact completion checks write, flush, measurement, read and close. Publication
uses its separate [transaction contract](native-publication.md).

Field/member correspondence uses exact resolved object identity and declared
slots, never matching approximate source text. Generated user-visible C names
come from full qualified category-separated registry keys. Runtime nominal type
numbers are local ordinals in this assembly's registry, not stable external ABI
identities. Native State and real C sizeof/_Alignof/offsetof provide truthful
payload layouts; each declared reference occupies a declared handle location.
Constants remain outside pools. Current mutable type/shared descriptors are
assembly globals, a known limitation awaiting the T061 context connection.
The [independent runtime foundation](execution-contexts.md) is tested separately.

Literal arrays preserve every source byte and explicit length, including zero.
The reserved byte used for an empty C array does not increase the Ama literal
length. Nested index accesses emit from inside out so dependencies precede their
users. A depth-D chain uses O(D squared) exact rescans and O(D) output, avoiding C
recursion. Source tables preserve original path and nominal name bytes through
fixed-width octal escapes. Declaration selectors start after allocated control
selectors, check exhaustion and match exact resolved declaration identity.

Owned native views follow the proven eager-layout order. Reference/list views
borrow declared locations without acquiring their targets. A normal binder checks
read eligibility; an interface binder relies on the already acquired invocation
and omits body-state fields. Borrowed views cannot outlive the operation or owner.

All Ama bodies become labels in one host execution function under
[STACK-04](../../spec/implementation/compiler.md#stack-04). Return links select
caller successors; saved positions have a separate switch checking nominal type
and permitted resume selector. Required control sites use exact resolved IR
queries; missing/duplicate sites fail emission. Current finite scans can be
quadratic in control/type counts; linear output size does not imply linear total
emission work. No guessed fallback selector or recursion-depth heuristic exists.

## Native verification and host policy

The project includes independent typed native declarations before authored bodies,
including the exact entry function type. Lifecycle/conversion adapters have their
actual signatures and const payloads for read-only protocols. Type-checking cannot
prove an arbitrary C body obeys borrowing, no-reentry or protocol-purity contracts;
those remain explicit native author obligations.

Compilation stages source files in a private temporary directory. Native commands
use argv with fork/exec, not a shell; wait retries only EINTR and requires normal
zero exit. Mandatory supported C11/O2/non-LTO options follow user compilation
flags. Linking puts user libraries after the verified object. The supported POSIX
nm record parser requires complete valid unstripped evidence including main and
rejects every exact registered native entry or supported compiler suffix family.
Only then may linking and executable retention succeed. This check does not
certify arbitrary flags, stripped/LTO workflows or a different toolchain.
Temporary cleanup is best effort; failed verification never authorizes publishing
an executable. See [project emission](project-emission.md).

The retained host boundary admits one project owner because mutable descriptors
are still shared. It rejects busy reentry, starts constants/root once, distinguishes
entry restart from resume, and reports terminal failure before disposing active
trace state. Yield returns an explicit resumable outcome. The standalone launcher
chooses to diagnose unhandled root yield, dispose and fail. This internal host API
is not the checked C-library ABI: independent contexts, boundary authentication,
compiled component exports and parallel admission remain scheduled work.

## Evidence

The per-file ready registry maps all retained driver/compiler/codegen inputs to
named tests, including the actual executable-only main adapter. Every header also
has an independent strict C11 syntax check. Request/options/output routing, frontend
and inspection failure prefixes, native authoring ownership, complete assembly
emission failures, required IR sites, literal ordering, source traces and explicit
host outcomes are tested in their real consumers. CLI integration invokes the
actual compiler and published program rather than a copied adapter.

All 43 formerly undocumented production inputs retain
identical compiler-lexed C against the preceding commit. The existing negative library cases additionally
check exact failing field types, exercising their previously unused shared diagnostic
checker and making the strict composition build warning-free. The driver/compiler and
codegen parents plus real CLI launcher/check/scaffold/execution suites validate
the composition. Doxygen validates file pages, source stability and canonical
rule destinations. These checks do not mark pending language features complete.
