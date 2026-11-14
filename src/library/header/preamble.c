/** @file
 * @brief Publish opaque host identities and separate API/technical execution channels.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) keeps rejected
 * C operations distinct from the three Ama outcomes and ordinary failure values.
 */
#include "internal.h"
void library_header_preamble(LibraryHeader *h) {
    const char *p = h->prefix, *m = h->macro_prefix;
    library_header_print(h,
        "/** @file\n * @brief Generated C11 consumer interface; no Ama/runtime/native-authoring layouts.\n"
        " * Context/instance values are copyable checked control identities, not addresses.\n"
        " * Owned snapshots/diagnostics have one releasing owner; C assignment is not cloning.\n"
        " * The host keeps component code loaded throughout every C call and owned resource.\n */\n"
        "#include <stddef.h>\n#include <stdint.h>\n#include <limits.h>\n"
        "#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L\n#error This consumer interface requires C11\n#endif\n"
        "#define %s_ABI_EDITION UINT32_C(1)\n"
        "#ifndef %s_REQUIRED_ABI\n#define %s_REQUIRED_ABI UINT32_C(1)\n#endif\n"
        "_Static_assert(%s_REQUIRED_ABI == %s_ABI_EDITION, \"Unsupported requested Ama C ABI edition\");\n"
        "_Static_assert(CHAR_BIT == 8 && sizeof(uint64_t) == 8 && sizeof(uintptr_t) == 8 && sizeof(size_t) == 8, \"Unsupported Ama C ABI integer/pointer model\");\n",
        m, m, m, m, m);
    library_header_identity_guard(h);
    library_header_print(h,
        "#if !defined(__x86_64__) || !defined(__linux__) || !defined(__ELF__) || !defined(__GNUC__)\n"
        "#error Unsupported Ama C consumer target/compiler profile\n#endif\n"
        "/** Caller/compiler default must match the declared platform C calling convention. */\n"
        "typedef void (*%s_DefaultCallingConvention)(void);\n"
        "/** This fixed target's System V convention is independent of -mabi overrides. */\n"
        "typedef void (__attribute__((sysv_abi)) *%s_RequiredCallingConvention)(void);\n"
        "_Static_assert(__builtin_types_compatible_p(%s_DefaultCallingConvention, %s_RequiredCallingConvention), \"Unsupported default C calling convention\");\n",
        p,p,p,p);
    library_header_print(h, "#define %s_COMPONENT_ID ", m);
    library_header_literal(h, h->build->component);
    library_header_print(h, "\n#define %s_TARGET ", m);
    library_header_literal(h, h->build->target);
    library_header_print(h, "\n#define %s_CALLING_CONVENTION \"c\"\n", m);
    static const char *const codes[] = {"OK", "INVALID_REQUEST", "BUSY", "REFUSED", "ABI_MISMATCH", "TECHNICAL_FAILURE"};
    for (size_t index = 0; index < sizeof(codes) / sizeof(*codes); index++) {
        library_header_print(h, "#define %s_API_%s UINT32_C(%zu)\n", m, codes[index], index);
    }
    library_header_print(h,
        "#define %s_COMPLETED UINT32_C(1)\n#define %s_YIELDED UINT32_C(2)\n#define %s_FAILED UINT32_C(3)\n"
        "/** Copyable opaque context identity. All-zero is empty; disposal invalidates all copies. */\n"
        "typedef struct { uint64_t opaque[2]; } %s_Context;\n"
        "/** Copyable export-bound instance identity. A handle does not keep its context alive. */\n"
        "typedef struct { uint64_t opaque[3]; } %s_Instance;\n"
        "/** Readonly counted bytes; no terminator, normalization or ownership transfer. */\n"
        "typedef struct { const uint8_t *data; size_t length; } %s_Bytes;\n"
        "/** One owned or component-pinned immutable diagnostic. Initialize empty and release once.\n"
        " * Message remains valid until release, including after context disposal.\n"
        " * Opaque words are control ownership, not Ama references or pool/trace addresses.\n */\n"
        "typedef struct { uint32_t code; const uint8_t *message; size_t length; uint64_t opaque[4]; } %s_Diagnostic;\n"
        "/** Separate API admission/result channel. A returned diagnostic transfers one owner.\n"
        " * API OK with execution FAILED is a technical Ama failure, not API rejection.\n"
        " * A context factory failure returns its diagnostic here without a context handle.\n */\n"
        "typedef struct { uint32_t code; %s_Diagnostic diagnostic; } %s_ApiResult;\n",
        m, m, m, p, p, p, p, p, p);
}
