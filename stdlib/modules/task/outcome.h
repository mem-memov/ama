/** @file
 * @brief Store one context-local terminal decision independently of callback publication.
 * [ASYNC-08](../../../spec/language/async-and-parallelism.md#async-08)
 * requires one explicit terminal commit that no later event can overwrite.
 */
struct AMA_NATIVE(State) { unsigned terminal; };
