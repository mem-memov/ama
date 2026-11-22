/** @file
 * @brief Capture complete prepared-image compiler inputs without physical-original authority.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires common final discovery before typing;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * passes exact prepared bytes through the independent captured-input compiler.
 * This owner cannot stand in for a physical SourceRootCapture in source selection.
 */
#ifndef AMA_PROJECT_SOURCE_IMAGE_CAPTURE_H
#define AMA_PROJECT_SOURCE_IMAGE_CAPTURE_H
#include "capture.h"
#include "../image/image.h"
/** Capture/catalog stages retain precise unavailable-versus-wrong-kind unit failures. */
typedef enum { SOURCE_IMAGE_CAPTURE_OK, SOURCE_IMAGE_CAPTURE_INVALID,
    SOURCE_IMAGE_CAPTURE_CATALOG, SOURCE_IMAGE_CAPTURE_MEMORY,
    SOURCE_IMAGE_CAPTURE_UNAVAILABLE, SOURCE_IMAGE_CAPTURE_KIND,
    SOURCE_IMAGE_CAPTURE_LIMIT } SourceImageCaptureStatus;
/** Own selected final catalog text, complete dot companions and snapshot tables.
 * Borrow the immutable image through teardown, including its explicit reader.
 * valid certifies common selection/topology and complete input capture only;
 * source syntax/types/operations, reference preservation and publication still
 * require the independent compiler and graph/site/transaction gates.
 */
typedef struct {
    ModuleCatalog catalog;
    SourceArena arena;
    SourceManager files;
    const SourceCatalogImage *image;
    const SourceCapturedUnit *units;
    DeliverySourceSnapshotView view;
    const char *failed_path;
    SourceStatus file_status;
    SourceCaptureStatus table_status;
    SourceImageCaptureStatus status;
    bool attempted, valid;
} SourceImageCapture;
/** Begin catalog, file and table owners under one borrowed allocator. */
void source_image_capture_init(SourceImageCapture *, SourceAllocator);
/** Release protocol/unit borrowers before their copied text/catalog owners. */
void source_image_capture_destroy(SourceImageCapture *);
/** Build once from one explicit immutable image using common complete package and
 * module discovery. Require every selected main and dot companion to have captured
 * regular-file metadata and bytes, including opaque native/unknown suffixes. A
 * missing row is absence only through the image's complete directory observations;
 * an existing unread member refuses rather than becoming empty/absent. Copy all
 * text under its final physical key; do not inspect C/native roles or execute code.
 * Reuse the physical capture's protocol-table constructor. Output is valid only
 * after all stages pass; failed scratch remains owned. No host filesystem reads,
 * source writes, acquisition or compiler/test execution. Repeat refusal preserves
 * the earlier result. Full semantic/input/reference/freshness checks remain required.
 */
SourceImageCaptureStatus source_image_capture_build(SourceImageCapture *, const SourceCatalogImage *);
#endif
