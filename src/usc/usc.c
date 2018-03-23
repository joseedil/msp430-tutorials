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
// This is done by means of the Power Management Module (PMM).
// Please refer to the device datasheet and family guide (slau208q) for details.
// Warning: potentially unsafe, do not use this in production systems!
uint16_t setVCoreUp (uint8_t level){
    uint32_t PMMRIE_bak, SVSMHCTL_bak, SVSMLCTL_bak;

    //Open PMM registers for write access
    PMMCTL0_H = 0xA5;

    //Disable dedicated Interrupts
    //Backup all registers
    PMMRIE_bak = PMMRIE;
    PMMRIE &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE |
		SVSLPE | SVMHVLRIE | SVMHIE |
		SVSMHDLYIE | SVMLVLRIE | SVMLIE |
		SVSMLDLYIE
		);
    SVSMHCTL_bak = SVSMHCTL;
    SVSMLCTL_bak = SVSMLCTL;

    //Clear flags
    PMMIFG = 0;

    //Set SVM highside to new level and check if a VCore increase is possible
    SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * level);

    //Wait until SVM highside is settled
    while ((PMMIFG & SVSMHDLYIFG) == 0) ;

    //Clear flag
    PMMIFG &= ~SVSMHDLYIFG;

    //Check if a VCore increase is possible
    if (PMMIFG & SVMHIFG == SVMHIFG){
        //-> Vcc is too low for a Vcore increase
        //recover the previous settings
        PMMIFG &= ~SVSMHDLYIFG;
        SVSMHCTL = SVSMHCTL_bak;

        //Wait until SVM highside is settled
        while (PMMIFG & SVSMHDLYIFG == 0) ;

        //Clear all Flags
        PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
		    SVMLVLRIFG | SVMLIFG |
		    SVSMLDLYIFG
		    );

        //Restore PMM interrupt enable register
        PMMRIE = PMMRIE_bak;
        //Lock PMM registers for write access
        PMMCTL0_H = 0x00;
        //return: voltage not set
        return ( -1 ) ;
    }

    //Set also SVS highside to new level
    //Vcc is high enough for a Vcore increase
    SVSMHCTL |= (SVSHRVL0 * level);

    //Wait until SVM highside is settled
    while (PMMIFG & SVSMHDLYIFG == 0) ;

    //Clear flag
    PMMIFG &= ~SVSMHDLYIFG;

    //Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;

    //Set SVM, SVS low side to new level
    SVSMLCTL = SVMLE | (SVSMLRRL0 * level) |
               SVSLE | (SVSLRVL0 * level);

    //Wait until SVM, SVS low side is settled
    while (PMMIFG & SVSMLDLYIFG == 0) ;

    //Clear flag
    PMMIFG &= ~SVSMLDLYIFG;
    //SVS, SVM core and high side are now set to protect for the new core level

    //Restore Low side settings
    //Clear all other bits _except_ level settings
    SVSMLCTL &= (SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 +
		 SVSMLRRL1 + SVSMLRRL2
		 );

    //Clear level settings in the backup register,keep all other bits
    SVSMLCTL_bak &=
        ~(SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 + SVSMLRRL1 + SVSMLRRL2);

    //Restore low-side SVS monitor settings
    SVSMLCTL |= SVSMLCTL_bak;

    //Restore High side settings
    //Clear all other bits except level settings
    SVSMHCTL &= (SVSHRVL0 + SVSHRVL1 +
		 SVSMHRRL0 + SVSMHRRL1 +
		 SVSMHRRL2
		 );

    //Clear level settings in the backup register,keep all other bits
    SVSMHCTL_bak &=
        ~(SVSHRVL0 + SVSHRVL1 + SVSMHRRL0 + SVSMHRRL1 + SVSMHRRL2);

    //Restore bak
    SVSMHCTL |= SVSMHCTL_bak;

    //Wait until high side, low side settled
    while (((PMMIFG & SVSMLDLYIFG) == 0) ||
           ((PMMIFG & SVSMHDLYIFG) == 0)) ;

    //Clear all Flags
    PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
		SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG
		);

    //Restore PMM interrupt enable register
    PMMRIE = PMMRIE_bak;

    //Lock PMM registers for write access
    PMMCTL0_H = 0x00;

    return ( 0 ) ;
}

uint8_t setVCore (uint8_t level){
    uint8_t actlevel;
    uint8_t status = 0;
    uint16_t interruptState;

    //Set Mask for Max. level
    level &= PMMCOREV_3;

    //Get actual VCore
    actlevel = (PMMCTL0 & PMMCOREV_3);

    //Disable interrupts because certain peripherals will not
    //work during VCORE change
    interruptState = __get_interrupt_state();
    __disable_interrupt();
    __no_operation();

    //step by step increase or decrease
    while ((level != actlevel) && (status == 0))
    {
        if (level > actlevel){
            status = setVCoreUp(++actlevel);
        } else   {
	  status = -1;
        }
    }

    //Re-enable interrupt state to whatever it was before
    if(interruptState & GIE)
    {
        __enable_interrupt();
    }

    return ( status) ;
}



void main(void)
{
  P4DIR |= BIT7; 	// Output pin.

  while(1){

    volatile unsigned int i;

    i = 10000;
    do i--;
    while(i != 0);

    P4OUT ^= BIT7;

  }
}
