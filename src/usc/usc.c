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



// Configure the Unified Clock System
void initClock(void)
{
  UCSCTL3 = SELREF_2;		   // Set DCO FLL reference = REFO

  __bis_SR_register(SCG0);	   // Disable the FLL control loop

  UCSCTL0 = 0x0000;		   // Set lowest possible DCOx, MODx
                                   // FLL later adjusts this value.

  UCSCTL1 = DCORSEL_7;		   // Select DCO range 50MHz operation
  UCSCTL2 = FLLD_0 + 762;	   // Set DCO Multiplier for 25MHz
				   // (N + 1) * FLLRef = Fdco
				   // (762 + 1) * 32768 = 25MHz
				   // Set FLL Div = fDCOCLK/1
  __bic_SR_register(SCG0);	   // Enable the FLL control loop

  // Worst-case settling time for the DCO when the DCO range bits have been
  // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  // UG for optimization.
  // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
  __delay_cycles(782000);

  // Set clock sources
  UCSCTL4 |=
    SELA_0 +		   // ACLK = XT1CLK = 32.768kHz (high precision)
    SELS_4 +		   // SMCLK = DCOCLKDIV = 25MHz
    SELM_4;		   // MCLK = DCOCLKDIV = 25MHz
}



void main(void)
{
  // Increase Vcore to maximum performance
  setVCore(0x3);

  // Try the demo also commenting the following line.
  //initClock();

  P4DIR |= BIT7; 	// Output pin.

  while(1){

    volatile unsigned int i, j;

    // Super slow loop
    j = 10;
    do
    {
      i = 35000;
      do i--;
      while(i != 0);

      j--;
    } while (j != 0);

    P4OUT ^= BIT7;

  }
}
