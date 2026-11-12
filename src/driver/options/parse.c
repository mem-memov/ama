/**
 * @file
 * @brief Compose operand checks, owned storage, option consumption and publication.
 *
 * [ARCH-01](../../../spec/implementation/compiler.md#arch-01) separates compiler
 * responsibilities; [ARCH-02](../../../spec/implementation/compiler.md#arch-02)
 * keeps this entry at orchestration level. No frontend or toolchain runs here.
 */
#include "options.h"
#include "storage/storage.h"
#include "validation/validation.h"
#include "assembly/consume.h"

bool driver_options_parse(DriverOptions *options, int count, char *const arguments[]) {
    options->request = driver_parse_request(count, arguments);
    if (!driver_options_operands(options, count, arguments)) {
        return false;
    }
    DriverOptionStorage storage = {0};
    if (!driver_options_allocate(options, (size_t)count, &storage)) {
        return false;
    }
    options->entry = arguments[3];
    options->roots[options->root_count++].path = arguments[2];
    if (!driver_options_consume(options, &storage, count, arguments) || !driver_options_validate(options)) {
        return false;
    }
    driver_options_publish(options, &storage);
    return true;
}
