/**
 * @file
 * @brief Admit native definitions using complete own static-formal identities, not placeholder spelling.
 *
 * [TPL-13](../../../../spec/language/templates.md#tpl-13) prohibits native
 * own parameters even when unused or bound at every application.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * the actual implementation marker and formal introduction in diagnostics.
 */
#ifndef AMA_STATIC_NATIVE_ADMISSION_H
#define AMA_STATIC_NATIVE_ADMISSION_H
#include "../inventory.h"

/** Own bounded diagnostic records, borrowing only Project/source data. The
 * input inventory need survive the call; Project/source must outlive this
 * result. No related pointer refers to a temporary or inventory arena record.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
typedef struct { Diagnostic error; DiagnosticRelated parameter; } StaticNativeAdmission;

/** Inspect an accepted inventory without mutation/allocation. On rejection,
 * publish the first native marker and its original ordinal-zero formal site.
 * On acceptance, leave output unchanged. Results/aliases/foreign residual slots
 * never count as owner inputs. Output owns its related record through rendering.
 * Contract: [TPL-13](../../../../spec/language/templates.md#tpl-13).
 */
bool static_native_admit(const StaticInventory *, StaticNativeAdmission *);
#endif
