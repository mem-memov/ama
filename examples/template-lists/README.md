# Generic list algorithms

These are ordinary Ama modules over the existing intrinsic list methods. They
use no action constants, custom allocator or replacement list provider. Types
are fixed during compilation; each ordinary instance keeps its own list state.

| Module | Input | Output and ownership |
|---|---|---|
| `reverse-owned` | `items[]` of any copyable element Type | Independent owning copies in reverse order. |
| `reverse-references` | `items&[]` of checked element references | Reversed memberships referencing the same targets. |
| `mirror` | `items[]` | `reverse(items)` followed by `items`, with independent owning copies. |
| `drain-reverse` | `items[]` | Reverse copies; consumes only its private input copy; returns `remaining = 0`. |
| `paired-reverse` | Two lists with independent element Types | Composes two ordinary reverse instances. |

For `[1, 2, 3]`, reverse returns `[3, 2, 1]`, mirror returns
`[3, 2, 1, 1, 2, 3]`, and drain-reverse returns `[3, 2, 1]` while its own
`items[]` becomes empty. The caller's original remains `[1, 2, 3]`: an owning
list input is copied normally. A reference result keeps aliases; changing a
live original target changes the value observed through those references.
Removing a reference-list membership does not destroy the referenced target.

## Read the code in this order

Start with `reverse-owned`: after k visits, its separate output contains the
reverse of the first k input entries. Then read `mirror`: inserting at zero
places each new membership before the current live boundary, so it is not
visited. Exactly the original n memberships are processed, and the result has
2n entries. `drain-reverse` copies an entry before deleting it and never reads
the removed reference again. Removing the current first entry corrects the
boundary so its surviving successor is still visited.

These examples deliberately expose live-boundary behavior. Inserting at zero
and removing from the front require shifting list order arrays. Their total
worst-case time is O(n²), not an optimized deque/reverse implementation; output
space is O(n). All copy/lifecycle work and refusal rules remain those of the
actual element Type. Exhaustion or a technical element-construction failure is
not converted into a successful partial answer.

## Static composition stays separate

`integer-family` binds only `paired-reverse`'s left element Type and exports a
Template with a residual right formal. `complete` accepts a Template and binds
its named right formal to native text. `assembly` explicitly connects these
static modules and also provides independent named integer/text reverse exports.
No residual Template is an object in the running program.

`main` uses the paired export. It retains `numbers = [3, 2, 1]` and
`words = [beta, alpha]` in its declared outputs and intentionally prints nothing.
From the repository root:

```sh
make
./ama --check examples/template-lists /test/fixture/v1.0.0/src/main --standard-root stdlib/modules
./ama --emit examples/template-lists /test/fixture/v1.0.0/src/main --standard-root stdlib/modules --out /tmp/ama-template-lists --sdk src --compile
/tmp/ama-template-lists/program
```

[The composition tests](../../tests/unit/codegen/template-algorithms/main.c)
load these exact maintained sources, compile real C executables and observe
actual integer/text values. They cover empty inputs, independent equal-key
instances, aliases, live insertion/deletion and invalid applications. The
[implementation record](../../docs/implementation/template-list-algorithms.md)
contains the proof, acceptance commands and limits.
