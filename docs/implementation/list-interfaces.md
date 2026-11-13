# Typed intrinsic list interfaces

T037 resolves the fixed service model of
[LIST-04](../../spec/language/lists.md#list-04),
[LIST-08](../../spec/language/lists.md#list-08) and
[LIST-09](../../spec/language/lists.md#list-09).
Execution and service-frame lowering remain later IR/runtime tasks. The compiler
continues to reject execution before publishing incomplete storage plans.

## One interface description for both semantic stages

The [immutable interface table](../../src/semantic/lists/interface.c) defines six
entries, their independent input/result sequences and exact base names:

| Entry | Inputs | Results |
| --- | --- | --- |
| `append` | `source&` | `element&` |
| `insert` | `position`, `source&` | `element&` |
| `remove` | `position` | None |
| `clear` | None | None |
| `swap` | `first_position`, `second_position` | None |
| `length` | None | `value` |

Element-reference fields specialize to the exact list element module. Other
fields specialize to the exact standard-library `..integer`, as owned scalars.
Input references default empty; omitted positions use zero. The owned length
result requires the typed size constructor. A placeholder still consumes its
independent interface position; an extra placeholder is an arity error.

Receiver inference and full binding checking use this same table. The compiler's
method enum identifies an interface during compilation; it is not an Ama action
constant or runtime dispatcher argument. Names such as `append` and `clear` remain
ordinary possible caller variable names outside the typed service selector domain.
Lookup compares complete bytes with no normalization or private-name fallback.

## Services are not hidden modules

[Service classification](../../src/semantic/lists/service.c) accepts a mutable
whole-list unmarked parent containing selected entry methods and no ordinary
inputs/outputs. A list does not expose retained results, execute all entries or
have a resume position. A service can originate in the caller or be selected as
a whole-list result from another operation.

[Target classification](../../src/semantic/calls/tree/target.c) distinguishes four call
kinds explicitly: module instance, constant namespace, intrinsic service and
intrinsic method. A service retains element/integer types and owning/reference
list kind. A method retains its service parent and fixed interface entry; it has
no nominal module type, private method state or fabricated field declaration.
Only `!` is valid for a method, without a target suffix or index.

Ordinary method bindings retain a static intrinsic descriptor instead of a
`SemanticSymbol`. Their type, form and ordinal are fully resolved. Diagnostics
for such interfaces use real operands, earlier bindings and actual nominal type
contracts, never an invented source declaration. Duplicates still fail by field
position/direction; ordinary source-declared duplicate diagnostics keep their
interface declaration and first-binding sites.

The method's outputs are ordinary typed values. [Shared result selection](../../src/semantic/selection/result.c) of `element&`
retains a scalar reference to the element type; selection of length's `value`
retains owned integer type. Such statements record their parent and intrinsic
result ordinal, not a fake caller field access. Their inputs and descendants use
the initiating caller's lexical environment under CALL-09. The ordered action
lists continue to represent complete subtrees before their next siblings.

## Explicit source versus omission

[Binding validation](../../src/semantic/lists/binding.c) first checks the normal
nominal/reference transfer rules. For owning `append`/`insert`, a supplied element
reference then requires the element type's static clone capability. This applies
even to a reference variable that is empty at runtime. A missing trailing source
or explicit empty positional placeholder requests default element construction
and requires no clone capability.

Reference-list methods copy the supplied handle rather than its referent. They
therefore accept noncopyable element types, including an empty/stale handle whose
runtime state is not guessed by static analysis. `remove`, `clear`, `swap` and
`length` do not produce element copies and introduce no element-clone requirement.
All six entries reject irrelevant parameter/result positions.

## The exact integer contract

[Integer validation](../../src/semantic/lists/integer.c) selects `..integer` by
its full identity; project `.integer` cannot substitute even with the same native
profile. The standard type must be native, independently copyable and expose its
literal/index protocols. Owned descendants participate in clone capability as
usual, so an advertised native `=` cannot conceal a noncopyable declared field.
Successful validation is cached in the semantic program without guessing payload
layout or current list size.

The type retains `requires_size_bridge` for
[PROTO-03](../../spec/implementation/native-interface.md#proto-03)'s fixed qualified
`from_size` declaration. Its actual authored payload type and callable C signature
must be checked by typed generated C at the native build boundary. Missing or
incompatible C declarations are build errors; semantic checking does not search
C text for a name, invoke a native constructor or add a seventh ABI protocol.
The static test fixtures deliberately need no callable C bodies. This task does
not certify the later list emitter or an authored integer's full numeric range.
T009 independently checks the typed runtime bridge boundary and failure cleanup.

## Invariants and bounds

After successful specialization, each method's type/form/ordinal comes from the
single immutable table and its particular service's exact element/integer types.
There is no generic method value that could accept unrelated parameters. Every
selected ephemeral instance has an explicit method parent/result ordinal, so it
cannot be confused with caller-owned storage. Every required copy is checked
before future runtime selection, and omission never fabricates a supplied source.

The fixed six-entry/two-input/one-output lookup is bounded independently of source
size and allocates nothing. Existing source-order, binding, action-link and
nominal lookup bounds apply; specialized records add O(B+M) storage for bindings
and method statements, with one cached integer contract per program. Allocation
failure leaves the program unresolved and every partial record arena-owned.

## Evidence

Run `make -C tests/unit/semantic/lists`.

Real two-root projects inspect all six interfaces, independent input/result
positions, named text order, exact inferred outputs, nullable declaration metadata
and method-free storage. Further calls on both ephemeral output kinds and services
selected from module results retain their exact parent/type/form. All eight
copy/literal/index profile subsets are checked; an Ama impostor, noncopyable
native descendant, project integer argument and absent standard root are rejected.

Tests distinguish omitted/default owning appends/inserts from explicit empty
reference sources, and show reference lists need no element cloning. They reject
invalid method modes/suffixes, irrelevant arguments, duplicate fields, whole-list
resume and retired dispatcher/read forms. Caller variables can reuse method names.
Every semantic allocation in a complete list-service project is independently
failed and counted ownership must return to zero. `make -C tests/unit/compiler/check`
proves successful static profile/bridge-obligation analysis without inspecting C,
and now complete T044 service/method IR. C generation retains the explicit
execution-capability guard until list adapters are implemented. Full suites and
sanitizer checks cover inference, ordinary bindings and structured plan composition.
