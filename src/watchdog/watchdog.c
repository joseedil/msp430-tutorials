/* Released under the MIT License */
/* Author: José Edil Guimarães de Medeiros <j.edil@ene.unb.br> */


#include <msp430.h>
#include <stdint.h>

// Disable the watchdog timer on the very beginning of the runtime code.
static void __attribute__((naked, section(".crt_0042"), used))
disable_watchdog (void)
{
  WDTCTL = WDTPW | WDTHOLD;
}


// Configure the watchdog in timer mode.
// ACLK is by deafult connected to the XT1 clock source. In the launchpad
// board, XT1 is driven by a high precision 32kHz crystal oscillator.
// The following configuration should provide a 250ms period timer.
inline void set_watchdog_timer(void){
  WDTCTL =
    WDTPW +		// Watchdog password
    WDTSSEL1 +		// ACLK = 32.768kHz
    WDTTMSEL +		// Interval timer mode
    WDTCNTCL +		// Clear
    WDTIS_5;		// Watchdog clk src/(2^13)

  SFRIE1 |= WDTIE;	// Enable WDT interrupt
}

void main(void)
{
  set_watchdog_timer();

  P4DIR |= BIT7; 	// Output pin.

  __enable_interrupt();
  while(1){}
}


// Watchdog interrupt handler
__attribute__((interrupt(WDT_VECTOR)))
void scheduler(void)
{
  P4OUT ^= BIT7;	// Toggle LED state.
}
