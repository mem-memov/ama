/**
 * @file
 * @brief Represent retained bounded size_t index payloads with checked decimal conversion in the native State layout.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the scalar library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 * [LIB-01](../../../spec/language/library.md#lib-01)
 * binds this native scalar by its explicit standard-root nominal identity.
 */
/**
 * Module-qualified native payload only; generated interface supplies AMA_NATIVE and required SDK
 * declarations. No persistent Ama identity may be hidden in this State.
 */
struct AMA_NATIVE(State) { size_t value; };
