# Repeating the written condition command

T068 executes ordinary branches, named loops and linear condition headers under
[FLOW-01](../../spec/language/control-flow.md#flow-01),
[FLOW-02](../../spec/language/control-flow.md#flow-02),
[FLOW-03](../../spec/language/control-flow.md#flow-03) and
[FLOW-04](../../spec/language/control-flow.md#flow-04).
The parser, semantic analysis and structured IR already define each loop's exact
root, body, exit and handler boundaries. C generation now executes those labels
without dropping header operations or retaining their grants in the body.

## A full header runs before each body iteration

Assume `object` returns a native `child` whose input/result implement decrement,
`remaining` is a caller value initially 3, and `probe` displays a value:

```text
object !
 child !
  < remaining
  output
  {
   object
    child
     output >> remaining
   probe !
    < remaining
  }
```

The header calls `object !` and then `child !` on each test. It converts `child`'s
stored result directly. It does not copy a condition to a hidden caller variable.
When the result is true, all header targets release before the body starts. The
body can therefore reacquire `object` and `child`, read the stored decrement into
`remaining`, and display 2, then 1. The next header returns false at 0 and exits;
that false result remains readable afterward. Ordinary output transfers belong
in separate operations or the body, as shown, rather than the linear header.

The compiled root also calls a native probe with persistent state. Its counter
advances three times, proving that the header repeats from `object !` rather than
only the nearest `child !`. Another case places the complete header inside a
branch inside a named loop. Native truth observations show that inner backedges
recheck neither the enclosing branch nor its outer loop.

## Convert within the correct owner, then release

The runtime's condition component borrows a selected scalar and invokes its
explicit native protocol under the existing world guard. Ordinary named
conditions use the caller as both operand domain and base owner. A header uses
the current protected outgoing target as the terminal result owner while keeping
index operands in the initiating caller. The helper performs no Ama dispatch,
implicit truth conversion or module-value copy.

An indexed terminal test gives the parent and caller different same-name indices.
The caller's decoder runs once and selects the true owned list element; the
parent's index would select false. This checks both the selection domain and
single protocol evaluation in the actual emitted program.

Truth is published only after checked protocol success. A failing callback may
write true to its scratch output, but neither the body nor any acquisition-refusal
handler runs. The original failure reaches terminal context cleanup. Existing
protocol tests retain the ordinary named-condition output-preservation contract.

Both successful truth successors go through explicit release-chain nodes. The
writer follows the terminal operation's fixed frame ancestry, releases selected
targets from deepest to shallowest, then releases the root. Successful conversion
requires every selected frame to be active; unlike refusal abandonment it cannot
silently tolerate an unentered link. The next label is the body or loop exit.

At either successor, the caller has no outstanding header target and every
header frame is inactive. Induction over the path establishes this from T066's
push/pop invariant. A body backedge reaches the one planned root label, so every
target, input and index is selected from current state again. No runtime search
for the nearest call or remembered target substitutes for that lexical boundary.

## Empty bodies and control exits

Parsed-empty named/header bodies check once for either true or false. Comment-only
bodies remain empty; source declarations remain nonempty even if they emit no
runtime instruction. Faithful truth instrumentation checks both values, and a
test-only watchdog bounds a broken repeated check instead of hanging the suite.
Its six-conversion bound comes from the largest test fixture; it is not a language
execution limit and does not change successful truth results.

Body `^` exits the innermost loop; body `>` completes the current invocation and
returns to its actual caller. Both start after header release. A body yield saves
its next ordinary statement; resumption continues there before the next backedge
starts the full header. Prefix/caller counters distinguish these transitions.

Root, intermediate and resume-link refusals release their complete acquired
prefix before their own handler. They skip terminal conversion and the unentered
body, and ancestor alternatives do not catch them. A `^` in such a handler uses
the enclosing ordinary body's loop context: the refused loop was never entered.
The compiled test exits that outer loop immediately without testing it again.

## Evidence and remaining adapters

`make -C tests/unit/codegen/loops` runs eight explicit cases comprising fifteen
compiled programs. They cover full/nested roots, both truth values for empty
named/header bodies, root/intermediate/resume refusal, incoming handler break,
failed truth, body break/return, body suspension and indexed terminal selection.
The existing `codegen/execution` suite checks standalone negative-only branches,
declaration-only bodies, ordinary condition errors and source diagnostics.
Sanitizer flags instrument the generated programs as well as their test runner.

The condition adapter adds no allocation or persistent activation. Releasing a
path of depth `d` performs `O(d)` checked frame transitions. Each uses a constant
number of registry/pool lookups; the current linear registry of `T` types makes
service work `O(d * T)`, excluding native-protocol costs. Storage is the already
planned fixed operation-frame area. Foreach cursors and intrinsic list-method
headers remain guarded until their own adapters are accepted; this task adds no
alternate source grammar or implicit condition protocol.
