#include "common/names.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static uint64_t hash_byte(uint64_t hash, unsigned char byte) {
    return (hash ^ byte) * UINT64_C(1099511628211);
}

static uint64_t hash_component(uint64_t hash, const char* text) {
    uint64_t length = (uint64_t)strlen(text);
    for (unsigned int index = 0; index < 8; index++) {
        hash = hash_byte(hash, (unsigned char)(length & 255));
        length >>= 8;
    }
    for (const unsigned char* byte = (const unsigned char*)text; *byte; byte++)
        hash = hash_byte(hash, *byte);
    return hash;
}

uint64_t source_identity_hash(const char* module, const char* member) {
    uint64_t hash = hash_byte(UINT64_C(14695981039346656037), member ? 2 : 1);
    hash = hash_component(hash, module);
    return member ? hash_component(hash, member) : hash;
}

void generated_name(char output[GENERATED_NAME_CAPACITY], GeneratedNameKind kind,
    const char* module, const char* member) {
    static const char* const prefixes[] = {
        "ama_state_", "ama_call_", "ama_instance_", "ama_field_", "ama_native_",
        "ama_parameter_", "ama_result_", "AMA_HEADER_"
    };
    snprintf(output, GENERATED_NAME_CAPACITY, "%s%016" PRIx64,
        prefixes[kind], source_identity_hash(module, member));
}

void emit_generated_name(FILE* output, GeneratedNameKind kind,
    const char* module, const char* member) {
    char name[GENERATED_NAME_CAPACITY];
    generated_name(name, kind, module, member);
    fputs(name, output);
}

void emit_source_label(FILE* output, const char* label) {
    fputs("/* Ama: ", output);
    for (const unsigned char* byte = (const unsigned char*)label; *byte; byte++) {
        if (*byte == '*' || *byte < 32 || *byte == 127)
            fprintf(output, "\\x%02x", *byte);
        else
            fputc(*byte, output);
    }
    fputs(" */\n", output);
}

static bool same_member(const char* left, const char* right) {
    return (!left && !right) || (left && right && strcmp(left, right) == 0);
}

bool name_registry_register(NameRegistry* registry, uint64_t hash,
    const char* module, const char* member, Error* error) {
    for (size_t index = 0; index < registry->count; index++) {
        const NameIdentity* previous = &registry->entries[index];
        if (previous->hash != hash) continue;
        if (strcmp(previous->module, module) == 0 && same_member(previous->member, member))
            return true;
        return error_set(error, ERROR_INVALID, 0,
            "generated name hash collision: '%s' member '%s' and '%s' member '%s'",
            previous->module, previous->member ? previous->member : "<module>",
            module, member ? member : "<module>");
    }
    if (registry->count == registry->capacity) {
        if (registry->capacity > SIZE_MAX / 2 / sizeof(NameIdentity))
            return error_set(error, ERROR_LIMIT, 0, "name registry capacity exceeded");
        size_t capacity = registry->capacity ? registry->capacity * 2 : 64;
        NameIdentity* entries = realloc(registry->entries, capacity * sizeof(*entries));
        if (!entries) return error_set(error, ERROR_MEMORY, 0, "out of memory (name registry)");
        registry->entries = entries;
        registry->capacity = capacity;
    }
    registry->entries[registry->count++] = (NameIdentity){hash, module, member};
    return true;
}

void name_registry_destroy(NameRegistry* registry) {
    free(registry->entries);
    *registry = (NameRegistry){0};
}
