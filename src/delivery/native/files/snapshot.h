/** @file
 * @brief Retain exact optional native-instruction and guidance observations.
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * fixes the selected companion origin; [BUILD-05](../../../../spec/implementation/application-builds.md#build-05)
 * distinguishes unavailable optional guidance from an absent document.
 */
#ifndef AMA_DELIVERY_FILE_SNAPSHOT_H
#define AMA_DELIVERY_FILE_SNAPSHOT_H
#include "source/path/exact.h"
#include "source/memory/arena.h"
#include "source/text/source.h"

/** Explicit observation tags retain presence separately from zero-length bytes
 * under [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * and unavailable guidance under [BUILD-05](../../../../spec/implementation/application-builds.md#build-05).
 */
typedef enum { DELIVERY_FILE_ABSENT, DELIVERY_FILE_PRESENT, DELIVERY_FILE_UNAVAILABLE } DeliveryFileState;
/** Arena-owned path and source-manager-owned bytes with an exact probe issue;
 * no text validation, CMake interpretation or alternate-copy lookup occurs under
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01).
 */
typedef struct {
    const char *path;
    const SourceText *content;
    DeliveryFileState state;
    PathProbe issue;
} DeliveryFileSnapshot;

/** Observe one exact child beneath an already verified stable directory. Both
 * owners must outlive the output; only a complete observation changes output.
 * Missing files and I/O/shape failures become distinct data, so the consumer
 * chooses the specified severity (required build input versus optional README).
 * Allocation/size failures return their SourceStatus and retain only owned
 * scratch. Bytes remain unchanged, including embedded zeros and invalid UTF-8.
 * O(D+B) time and O(path+B) retained space for D directory entries and B bytes.
 * [BUILD-05](../../../../spec/implementation/application-builds.md#build-05)
 * makes guidance failure nonfatal; [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * still requires a usable authored companion when supplied.
 */
SourceStatus delivery_file_capture(SourceArena *, SourceManager *, const char *parent,
    const char *name, DeliveryFileSnapshot *);
#endif
