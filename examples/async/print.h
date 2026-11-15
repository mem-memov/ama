/** @file
 * @brief Keep the synchronous output command resource-free and retain no borrowed text view.
 * [CABI-05](../../spec/implementation/native-interface.md#cabi-05)
 * gives its text parameter ordinary declared Ama ownership.
 */
struct AMA_NATIVE(State) { unsigned char unused; };
