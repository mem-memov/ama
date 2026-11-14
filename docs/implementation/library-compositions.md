# Arithmetic outcomes and shared-list compositions

T087 composes the current library through ordinary module interfaces. It applies
[ERROR-01](../../spec/implementation/diagnostics.md#error-01)'s failure convention
without changing intrinsic list refusals or normal false predicates. It also
accepts list-valued constants through the existing context-local construction,
shared-to-owned recipe and cleanup machinery.

## Predictable results and distinct outcomes

| Operation | Inputs | Results | Expected failure |
|---|---|---|---|
| `..std.index.next` | value | result, failure | Host-size successor overflow |
| `..std.index.add` | left, right | result, failure | Host-size addition overflow |
| `..std.index.before` | left, right | result | None; false is ordinary data |
| `..integer.next` | value | result | None; arbitrary precision |
| `..integer.add` | left, right | result | None; arbitrary precision |
| `..integer.before` | left, right | result | None; false is ordinary data |

Value inputs/results are owned and retain their exact declared nominal types.
Comparison results are `..std.condition`; failure results are `..failure` with
its owned public text description. Named bindings may appear in caller text
order; parameter installation still follows declaration order. Positional
omissions keep their established behavior.

Bounded overflow returns technical success with a true failure and a current
description. The numeric result remains its reset default zero and is not a
successful arithmetic result. A later successful call establishes false/empty.
Allocation/share/preparation failures remain technical and do not enter `:`.
The arithmetic guards perform no overflowing operation: successor checks
`value == SIZE_MAX`, and addition checks `left > SIZE_MAX - right` before adding.

Intrinsic list methods retain their own typed preflight/refusal contract. This
change adds no list action constants, result outcome enums, Boolean companions
or parser-recognized arithmetic names. A refused ordinary call transfers no
outputs, so receivers retain their earlier values. A handler must not process
such an old failure as a newly produced result.

## Exact signed arithmetic stays in the integer library

[Native arithmetic](../../stdlib/modules/integer+/native/arithmetic.c) extends
the canonical integer's public typed C helper interface. Ordinary constructor
modules at `integer+/add`, `next` and `before` use these helpers; generated Ama
code still knows neither limb layout nor arithmetic operators.

Canonical magnitudes have at least one little-endian base-256 limb, no leading
zero limb, and no negative zero. Their magnitude order is limb-count order,
then the most significant differing limb. Different signs determine order
directly; negative values reverse magnitude order. Comparison publishes only
-1/0/1 after validating live sources.

For same-sign addition, allocate one limb beyond the larger magnitude. Each
column computes `a + b + carry <= 511`; quotient/remainder by 256 give an exact
carry of zero or one. The extra column consumes the final carry. For differing
signs, compare magnitudes, subtract the smaller from the larger, and retain the
larger's sign. Each column uses an unsigned borrow of zero/one; adding 256
before subtraction keeps every intermediate nonnegative. Inductively each
processed prefix represents exactly the corresponding mathematical prefix plus
its outgoing carry/borrow. Normalization removes leading zero limbs and makes
cancellation ordinary nonnegative zero.

Work and prepared storage are O(L) in the larger limb count. Checked count
arithmetic precedes allocation. Preparation reads both complete operands before
finalizing the old result, so left/right/both destination aliases are supported.
Allocation failure preserves sources and the old result. Success transfers one
prepared native ownership credit; there is no signed host narrowing or fixed
mathematical overflow. Native helpers may use the C stack, but all Ama invocation
continues to use generated label dispatch.

## List constants use the same context-owned lifecycle

[Constant initialization](../../src/runtime/program/constants/build.c) allocates
one context-owned array per declared constant, constructs each element in order,
and records partial storage before continuing. Nested owned list fields use the
same iterative child construction. Constant-to-constant copying follows the
complete immutable ownership graph; shared-to-owned recipes create independent
Ama instances and preserve exact nested list shapes.

The composed tests use real immutable text backings. Copying an owned text list,
clearing/appending its mutable copy, or disposing one context leaves the source
constants and another context untouched. There are no default namespace instances
or source-body execution during startup. A failed later constant element cleans
its completed prefix before root publication, preserving the primary status.
No new alternative constant representation or list snapshot was introduced.

The two-live-context probe holds explicit inspection leases, verifies distinct
constant arrays and distinct native byte backing, closes the leases, disposes
one suspended context, then resumes the other. The nested probe initializes a
scalar bundle with an owned text list, uses it to initialize a list of constant
bundles, copies that list into owned storage and reads the public copied lists.

## Evidence and remaining boundaries

The [per-file inventory](documentation/ready.json) records the actual native,
interface, fixture and Makefile consumers and standalone header probes.

- `make -C tests/unit/native/abi/integer-arithmetic` compares complete bounded
  sign/order combinations and selected limb boundaries with independent host
  arithmetic, then uses decimal expected values beyond every C signed range.
  It checks cancellation, left/right/both aliases and preparation failure.
- `make -C tests/unit/codegen/library-compositions` executes bounded outcome
  transitions, unchanged receivers after refusal, arbitrary-precision typed
  arithmetic, ordinary false predicates, mutable copies of shared text lists,
  two live contexts, nested list-valued constants and startup-prefix failure.
- `make -C tests/unit/codegen/library` retains ordering/default/comparison
  composition acceptance for the bounded scalar library.
- Native/generated ASan/UBSan, strict whole-project tests, complete per-file
  coverage, local links and reproducible Doxygen validate the final change.

These interfaces do not implicitly convert `..integer`, `..std.index` and
`..std.condition`. File/I/O and higher-level examples remain subsequent tasks;
no public operation infers failure by inspecting message wording.
