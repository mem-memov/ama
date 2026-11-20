/** @file
 * @brief Own no callback in the Ama payload; the independently resident backend owns every submitted native job.
 * [CABI-05](../../../../../../../../spec/implementation/native-interface.md#cabi-05)
 * keeps the operation reference in declared Ama interface storage.
 */
struct AMA_NATIVE(State) { unsigned char unused; };
