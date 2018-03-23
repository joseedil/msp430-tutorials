#ifndef __PMM_H__
#define __PMM_H__

#include <msp430.h>
#include <stdint.h>

extern uint16_t setVCoreUp (uint8_t level);
extern uint8_t setVCore (uint8_t level);

#endif // __PMM_H__
