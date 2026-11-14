/** @file
 * @brief Keep the failure constructor resource-free; declared fields own text/outcome values.
 * [ERROR-01](../../../spec/implementation/diagnostics.md#error-01) returns one complete outcome.
 */
struct AMA_NATIVE(State) { unsigned char reserved; };
