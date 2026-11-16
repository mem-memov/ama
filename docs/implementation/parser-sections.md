# Module activation and sections

Boundary: `src/parser/module/sections.*`, retaining section/activation structure
for SYN-03/04/05; shared lexical classification and declaration-region integration are completed in T023.
Section views borrow the immutable layout. Recognition allocates nothing and
publishes output only on success. Global UTF-8/TAB/line-ending checks must already have passed layout reading.

A first pass activates only exact zero-indent `@=`, `@!` or `@C` code lines, with
the classified trailing comment suffix. Literal/comment text and indented markers
never activate a draft. A nonactivated file remains a draft without parsing its
unfinished declarations. The shared classifier validates lexical joins even when declarations remain unfinished.

The second pass assigns increasing section ranks: constants, parameters, results,
implementation. Duplicate/decreasing/unknown/indented annotations fail at their
source range. Nonlayout before the first section or after `@C` fails. Only a
constants section can finish without an implementation; interfaces require one.
Each section is a half-open interval of layout nodes, keeping declaration/body
validation separate for the following AST task.

The rank invariant proves allowed order and uniqueness by induction over markers.
The current section owns each non-marker line until the next marker, so intervals
partition section content without interpreting names. Both finite passes advance
one line each iteration: O(lines) time, constant storage.
Errors include an exact primary span and a stable ParseError grammar expectation;
see the [error contract](parser-errors.md).

Evidence: `make -C tests/unit/parser/module` passed C11/pedantic/Werror. Cases cover
unfinished drafts, false activation inside payload/comments, empty constants and
bodies, both implementations, missing implementations, ordering, duplicates,
unknown/indented annotations and nonlayout native content. This component alone
does not claim that declaration/body grammar has been validated.


T146 adds Doxygen contracts for the local rank pass and borrowed section intervals.
Global encoding, TAB and line-ending checks run in the physical reader. T023
then applies common tokens before activation and checks declaration regions;
see [the composition contract](declaration-parsing.md). Unfinished draft grammar
remains excluded without an alternate lexical spacing or TAB policy.
