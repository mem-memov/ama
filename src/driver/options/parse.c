/**
 * @file
 * @brief Compose operand checks, owned storage, option consumption and publication.
 *
 * [ARCH-01](../../../spec/implementation/compiler.md#arch-01) separates compiler
 * responsibilities; [ARCH-02](../../../spec/implementation/compiler.md#arch-02)
 * keeps this entry at orchestration level. No frontend or toolchain runs here.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) keeps source/--root
 * entries in the project/import family; standard authority is never guessed from paths.
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
    if (options->request == DRIVER_REQUEST_LIBRARY_CHECK || options->request == DRIVER_REQUEST_LIBRARY_HEADER) { options->library_configuration = arguments[3]; }
    else { options->entry = arguments[3]; }
    options->roots[options->root_count++] = (ModuleRoot){
        .path = arguments[2], .kind = MODULE_ROOT_PROJECT};
    if (!driver_options_consume(options, &storage, count, arguments) || !driver_options_validate(options)) {
        return false;
    }
    driver_options_publish(options, &storage);
    return true;
}
