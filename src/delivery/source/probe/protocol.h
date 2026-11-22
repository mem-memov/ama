/** @file
 * @brief Share exact read-only entry-probe completion states without compiler owners.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps project consumers independent of frontend state;
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) distinguishes executable
 * tests from genuinely open helpers without diagnostic guessing.
 */
#ifndef AMA_DELIVERY_SOURCE_PROBE_PROTOCOL_H
#define AMA_DELIVERY_SOURCE_PROBE_PROTOCOL_H
/** A normal probe result is distinct from metadata validity and child failure.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) does not treat Open as an executable test.
 */
typedef enum { DELIVERY_PROBE_NONE, DELIVERY_PROBE_EXECUTABLE, DELIVERY_PROBE_OPEN } DeliverySourceProbeState;
/** Dedicated captured-probe command completion: zero is executable, ten is a
 * proved open original root; all failures use normal failure status. Only the
 * explicit --probe-entry mode interprets ten as a completed classification under
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
enum { DELIVERY_SOURCE_PROBE_OPEN_EXIT = 10 };
#endif
