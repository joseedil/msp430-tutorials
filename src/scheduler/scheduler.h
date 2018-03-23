#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <msp430.h>
#include <stdint.h>

#define BACKUP_REGS 12 /* r4 to r15*/

#define STACK_SIZE  1024
#define STACK_TOP   STACK_SIZE - 1
#define TOTAL_TASKS 3

/*Enable GIE in SR so that the WDT never stops when we go to user task*/
/*Enable SCG0 for 25MHZ CPU execution*/
#define DEFAULT_SR  ((uint16_t)0x0048)

#define SAVE_CONTEXT()           \
  asm volatile ( "push r4  \n\t" \
                 "push r5  \n\t" \
                 "push r6  \n\t" \
                 "push r7  \n\t" \
                 "push r8  \n\t" \
                 "push r9  \n\t" \
                 "push r10 \n\t" \
                 "push r11 \n\t" \
                 "push r12 \n\t" \
                 "push r13 \n\t" \
                 "push r14 \n\t" \
                 "push r15 \n\t" \
               );

#define RESTORE_CONTEXT()       \
  asm volatile ( "pop r15 \n\t" \
                 "pop r14 \n\t" \
                 "pop r13 \n\t" \
                 "pop r12 \n\t" \
                 "pop r11 \n\t" \
                 "pop r10 \n\t" \
                 "pop r9  \n\t" \
                 "pop r8  \n\t" \
                 "pop r7  \n\t" \
                 "pop r6  \n\t" \
                 "pop r5  \n\t" \
                 "pop r4  \n\t" \
                 "reti    \n\t" \
               );


#endif // __SCHEDULER_H__
