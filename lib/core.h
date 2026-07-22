/* date = July 21st 2026 */

#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#define internal       static /* File-local linkage for functions / variables */
#define local_persist  static /* Local scope with permanent program lifetime */
#define global         static /* Translation-unit scope global variable */

/* 
 * Universal Debug Assertion Macro
 * Triggers a hardware breakpoint (bkpt #0) when an invariant fails.
 */
#define Assert(expr) do { if (!(expr)) { __asm volatile("bkpt #0"); } } while(0)

/* 
 * ARM Core Memory Barrier Instruction
 * Ensures memory operations complete before execution continues.
 */
#define __DMB() __asm volatile("dmb" ::: "memory")

#endif /* CORE_H */
