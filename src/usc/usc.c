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


// To achieve maximum performance, we should increase the supply voltage.
// This is done by means of the Power Management Module (PMM). We set the
// supply voltage to the maximum setting. Please refer to the device
// datasheet and family guide (slau208q) for details.
void SetVCoreUp (unsigned int level)
{
    // Open PMM registers for write access
    PMMCTL0_H = 0xA5;
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    if ((PMMIFG & SVMLIFG))
      while ((PMMIFG & SVMLVLRIFG) == 0);
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}

void set_pmm(void){
  SetVCoreUp(1);
  SetVCoreUp(2);
  SetVCoreUp(3);
}


void main(void)
{
  set_pmm();

  P4DIR |= BIT7; 	// Output pin.

  while(1){

    volatile unsigned int i;

    i = 10000;
    do i--;
    while(i != 0);

    P4OUT ^= BIT7;

  }
}
