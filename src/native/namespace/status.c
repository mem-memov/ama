/**
 * @file
 * @brief Emit one preplanned component-local native error namespace in a contained C context.
 *
 * [ERROR-04](../../../spec/implementation/diagnostics.md#error-04) prevents
 * collisions between providers. [NAME-05](../../../spec/implementation/native-interface.md#name-05)
 * contains companion macros; ordinary C declarations are neither parsed nor rewritten.
 */
#include "namespace.h"

/** Emit only a validated nonzero namespace. Each macro argument is passed once
 * to the allocation-free SDK status constructor; end-of-context removes the macro.
 * Invalid arguments fail before output and IO failure leaves an unusable partial artifact.
 * Component contract: [ERROR-04](../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../spec/implementation/native-interface.md#name-05). */
bool native_namespace_status(FILE *output, uint16_t namespace_id) {
    if (output == NULL || namespace_id == 0) { return false; }
    return fprintf(output, "#include \"native/status/library.h\"\n"
        "#ifndef AMA_NATIVE\n#error A native error provider requires an active naming context\n#endif\n"
        "#ifdef AMA_NATIVE_ERROR\n#error A native error provider is already active\n#endif\n"
        "#define AMA_NATIVE_ERROR(reason,message,length) ama_library_status(UINT16_C(%u),(reason),(message),(length))\n",
        (unsigned)namespace_id) >= 0;
}
