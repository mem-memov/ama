# Conditions, branches, and loops

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Choosing a branch](#choosing-a-branch)
- [Repeating a body](#repeating-a-body)
- [Which values can be conditions or indices?](#which-values-can-be-conditions-or-indices)
- [Computing a loop condition through calls](#computing-a-loop-condition-through-calls)

<!-- /chapter-contents -->

Ama tests named values rather than evaluating general expressions. A type's
native `?` conversion says how one of its values becomes a boolean condition.
The `?` and `:` markers select branches; braces describe repetition.

The examples in this chapter are body fragments with previously declared
operands. List traversal has its own header and is explained in
[Working with lists](lists.md#traversing-a-changing-list).

## Choosing a branch

<a id="flow-01"></a>

**FLOW-01.** A named owned condition admits three branch forms: `?` with a
positive body, `?` with a positive body and a following `:` negative body, or
`:` alone with a negative body. The last form executes its body only when the
condition is false; it requires neither a `?` marker nor an empty positive body.
The condition and its markers occupy separate lines at equal indentation.
Only comment lines at the condition's indentation may intervene between it and
its first marker; blank lines may not. Each body may be empty under SYN-01; any statements in it
are one level deeper. A negative-only branch cannot be extended by another `?` or
`:` marker; a later independent branch requires its own condition. A `:` without
a preceding condition or eligible call/read is not a statement.

On each entry to the branch, evaluate the condition's read-only native `?`
conversion exactly once and execute the body selected by that boolean result,
or skip the branch when that body is absent. The negative form changes only
which boolean result selects its body; it does not modify the condition or
introduce another conversion protocol. A technical conversion failure remains
fatal, never a false result or an acquisition refusal caught by `:`. Resuming
from a saved position inside a selected body continues there without reevaluating
the condition; reaching the branch anew evaluates it again.
An empty selected body performs no statements and continues after the branch;
even when both bodies are empty, the condition is still evaluated as specified.

```text
is ready
:
 handle not ready !
```

This conditional `:` is distinct from a call/read acquisition-refusal handler.
Its role follows the operation's syntax, not the declared type or a lookup-based
choice; COMP-01 requires structural recognition.

### Example: positive and negative outcomes

Assume `is ready` has a valid truth conversion, while `start work` and
`report delay` are callable instances.

```text
is ready
?
 start work !
:
 report delay !
```

The condition is converted once. Exactly one of the two calls executes. If only
the negative outcome needs work, the earlier negative-only example expresses it
without an empty positive branch. Both examples select caller-owned instances;
they do not declare variables named after the branch markers.

## Repeating a body

<a id="flow-02"></a>

**FLOW-02.** A named condition followed by `{` begins a condition-controlled loop;
`}` ends it. For a nonempty body, evaluate the condition before each iteration:
true enters the body and false continues after `}`. `^` exits the innermost loop
and is invalid outside a loop. A standalone `>` completes the current invocation
early; reaching the body end also completes it.
FLOW-05's reference-binding header selects a distinct list-iteration statement;
its iteration does not use a truth conversion.

For an empty body under SYN-01, evaluate the condition access exactly once on
each entry to the loop and, if access succeeds, perform its native `?` conversion
exactly once. Either successful boolean result continues immediately after the
matching `}`; do not repeat the check or yield. Access and conversion failures
retain their ordinary failure behavior, never becoming false or successful
completion. The check and its failures must be preserved even though its boolean
result does not affect the continuation target.

Emptiness is determined from parsed statements before lowering/optimization.
Comments and blank lines do not make the body nonempty; a declaration or nested
control statement does, even if it emits no runtime work. The empty-body warning
and required closing `}` remain in effect.

```text
is ready
{
 # loop body pending
}
continue work !
```

Here `is ready` is checked once, then `continue work !` executes for either truth
value, provided the check succeeds.

## Which values can be conditions or indices?

<a id="flow-03"></a>

**FLOW-03.** Truth and index conversions use explicit standard library contracts
mapped by the compiler. Their signatures and host-size conversions must be
checked. No universal nonzero-integer truth rule is built into the language.
The condition/index operand is a named owned scalar access, possibly an indexed
owned element; a loop may select its condition directly from a result under
FLOW-04. Its own native type must bind `?`/`[]` respectively. These
capabilities are not implicitly inherited from an owned field. An Ama composition
can call library operations and use their typed results as operands; it does not
gain another truth/index protocol or a general expression evaluator.

## Computing a loop condition through calls

**Why this form exists.** An operation often returns the next condition. The
chain below uses that result where it is stored and repeats the exact written
condition command, without introducing a temporary field.

<a id="flow-04"></a>

**FLOW-04.** A loop condition may be a linear chain of instance operations
ending in a named owned scalar result, without a transfer into caller storage.
The entire chain and terminal loop form one ordinary statement. For example:

```text
a
 b !
  condition
  {
   step !
  }
```

Here `a` reads its retained results, `b !` executes its declared result `b`, and
`condition` selects a declared owned scalar `@>` result of `b`. Apply the selected
value's native `?` conversion; do not execute its body, copy it, or infer a local
field. Root and intermediate targets may use the existing owned/reference/indexed
operation forms, with exactly the declared storage suffixes. A terminal indexed
owned result element is allowed; a reference, whole list, module path, or a type
without its own truth capability is not a condition under FLOW-03.

Only a loop recognized as a statement in an ordinary body starts a new loop
context. Recursive parsing of its header links continues the same loop; fold
those links, the terminal condition, and its body into one loop AST node. A new
loop statement inside that body or a handler is a separate nested loop.

### One path through declared results

The header is a single path with exactly one terminal condition. Each operation
link has one successor one level deeper. Entry links may precede it with ordinary
input bindings; resume and retained-read links accept no inputs. Ordinary output
bindings and sibling output actions are not part of this condition-chain form;
write other operations as separate statements or in the loop body. Each link
retains its explicit mode: `!` starts from entry, `%` resumes without inputs,
and an unmarked link reads retained results without executing its body. No mode
is inferred from the presence of `{`.

The root target resolves in the initiating caller. Every subsequent target,
including the terminal condition, resolves only in the preceding operation's
declared `@>` interface or LIST-04's intrinsic service/method result interface
under CALL-09. A list-service link is bare and selects a method link with `!`;
there is no list dispatcher entry, method resume, or retained method read.
Input sources and index operands resolve
in the initiating caller, as do ordinary statements in the loop body and refusal
handlers. Result-selector base names are excluded from TYPE-06's local-name scan;
the root target and local names in inputs/indices still participate. Statically
validate the complete header and body before execution, independently of lookup
choices during parsing.

### Where every iteration starts

The repeat boundary is exact and lexical:

1. Identify the complete loop statement in its containing ordinary body. Its
   first target is the root at that body's statement indentation. Assign this
   loop a fixed condition-entry label at that root, a body-entry label, and an
   exit label after the complete statement. Each nested loop has its own labels.
   The condition-entry label never belongs to an enclosing branch, loop, or
   dynamically calling module. A named condition with no preceding links uses
   the same rule with its own access as the condition entry under FLOW-02.
2. On initial entry or a loop backedge, begin at that condition-entry label.
   Resolve the root and every selected target/index again from current caller
   state. Execute all explicitly written calls/resumes in header order under
   CALL-08/CALL-09, applying their ordinary interface and persistent-state rules.
   Do not cache an earlier target identity, result, index, or prefix across body
   execution. A bare `a` does not execute `a`; an explicit `a !` executes on every
   test. A constructor intended to run once belongs in a preceding statement.
3. Keep the necessary header storage protected through terminal selection and
   its single native truth conversion. After successful conversion, release all
   this header's acquisitions, current return connections, and chain service
   frames before entering the body or leaving the loop. Preserve ordinary
   persistent state and modules' own saved positions. The body is not a pending
   output action and retains no header protection.
4. If the result is false, continue at this loop's exit label. If true and the
   body is nonempty, execute its ordinary caller-side statements. Reaching its
   matching `}` returns to this loop's condition-entry label. `^` in this body
   exits this loop; `>` completes the invocation. Suspension saves the body
   position under CONT-02, and resumption continues there without replaying the
   header; later reaching `}` begins the next test. No test of an enclosing
   branch or loop is implicitly repeated by this backedge.
5. If the terminal loop body is empty under SYN-01, evaluate the entire header
   only once on each entry and continue at the exit label for either successful
   boolean result under FLOW-02. Header inputs and refusal-handler statements
   do not make the loop body nonempty. Emit its warning at `{`.

The terminal condition and `{` align and admit only intervening comment lines
at that same indentation, as for a named condition. The body is one level below
`{`, and `}` aligns with
it. Completing this statement returns to the root's containing body; a following
ordinary statement starts at the root's indentation, not at an intermediate
selector level. Unique successors and the containing ordinary body determine
one root and one terminal condition at any finite source depth, so the repeat
label requires no heuristic search for a nearby `!`, marker, or runtime caller.

### Refusal handlers and nested loops

An operation link may have its own acquisition-refusal `:` handler at that
link's indentation after its successor's complete syntax. On refusal, apply
CALL-09 cleanup and execute only that link's handler. Normal handler completion
continues at this loop statement's exit label, never retries the test or treats
refusal as a false condition. An absent handler uses the ordinary unhandled
failure path. Terminal access/conversion failures retain FLOW-02/FLOW-03 and
PROTO-05 behavior; a link handler does not catch technical failures.

Header-link handlers use the enclosing ordinary body's loop context, excluding
the loop whose test was refused: a `^` there exits an enclosing outer loop or is
invalid when none exists. A nested loop in such a handler supplies its own break
context normally. A handler suspension resumes in that handler and then exits
the refused loop statement; no discarded header work is restored.

For example, nested tests have independent boundaries:

```text
a
 b !
  outer condition
  {
   enabled
   ?
    c
     d !
      inner condition
      {
       step !
      }
   after inner !
  }
after outer !
```

The inner backedge begins at `c`, then executes `d !` and checks its result. It
does not restart `enabled`, `a`, or `b !`. After the outer body completes, the
outer backedge begins at `a` and executes `b !` again. Calls made inside `b` or
`d` belong to those modules' own implementations and cannot change these lexical
repeat boundaries. Between tests and throughout a loop body, no service frame
or protection from that loop's completed header remains.

### Example: a constructor outside the repeated command

Assume `factory !` creates its declared method result `next`, and that `next !`
returns a truth-capable owned result `has item`.

```text
factory !
factory
 next !
  has item
  {
   process item !
  }
```

The constructor executes once. Each test then reads `factory`'s retained results,
executes that same `next` result, and converts `has item`. The loop body executes
while that conversion is true. A test repeats from the unmarked `factory` line;
it does not replay the preceding constructor call. `process item` is resolved in
the caller, as are other ordinary statements in the body.

`^` and `>` leave control structures as described above. A standalone `%`
instead suspends this invocation; see [Continuations](continuations.md) for the
saved-position and resumption rules.
