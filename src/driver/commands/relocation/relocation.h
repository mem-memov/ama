/** @file
 * @brief Run explicit relocation preview/application/recovery commands with durable outcome reporting.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * separates preview from apply; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains recovery data and never describes a committed move as rolled back after output failure.
 */
#ifndef AMA_DRIVER_RELOCATION_H
#define AMA_DRIVER_RELOCATION_H
#include <stdio.h>
#include "driver/options/options.h"
/** Borrow validated options/streams through a complete single attempt; destroy operation owners only after diagnostics/reporting. */
int driver_relocation(const DriverOptions *, FILE *, FILE *);
#endif
