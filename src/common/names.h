#ifndef AMA_COMMON_NAMES_H
#define AMA_COMMON_NAMES_H

#include <stdint.h>
#include <stdio.h>
#include "common/diagnostic.h"

#define GENERATED_NAME_CAPACITY 64

typedef enum {
    NAME_STATE, NAME_CALL, NAME_INSTANCE, NAME_FIELD, NAME_NATIVE,
    NAME_PARAMETER, NAME_RESULT, NAME_HEADER
} GeneratedNameKind;

/* Hash: 64-bit FNV-1a. Identity: domain byte, then length-prefixed UTF-8 module/member strings.
    Each byte length is encoded as eight little-endian bytes.
    NULL member means a module identity; empty member is a distinct identity.
    Names are independent of source enumeration order and host byte order.
*/
uint64_t source_identity_hash(const char* module, const char* member);
void generated_name(char output[GENERATED_NAME_CAPACITY], GeneratedNameKind kind,
    const char* module, const char* member);
void emit_generated_name(FILE* output, GeneratedNameKind kind,
    const char* module, const char* member);
void emit_source_label(FILE* output, const char* label);

typedef struct {
    uint64_t hash;
    const char* module;
    const char* member;
} NameIdentity;

typedef struct {
    NameIdentity* entries;
    size_t count;
    size_t capacity;
} NameRegistry;

/* Entries borrow source strings until destroy. The caller supplies the computed
    hash; explicit hashes also permit deterministic collision-path testing.
*/
bool name_registry_register(NameRegistry* registry, uint64_t hash,
    const char* module, const char* member, Error* error);
void name_registry_destroy(NameRegistry* registry);

#endif
