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
// Warning: potentially unsafe, check it before use in production systems!
uint16_t setVCoreUp (uint8_t level){
    uint32_t PMMRIE_bak, SVSMHCTL_bak, SVSMLCTL_bak;

    //Open PMM registers for write access
    PMMCTL0_H = 0xA5;

    //Disable interrupts and backup registers
    PMMRIE_bak = PMMRIE;
    PMMRIE &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE |
		SVSLPE | SVMHVLRIE | SVMHIE |
		SVSMHDLYIE | SVMLVLRIE | SVMLIE |
		SVSMLDLYIE
		);
    SVSMHCTL_bak = SVSMHCTL;
    SVSMLCTL_bak = SVSMLCTL;

    //Clear interrupt flags
    PMMIFG = 0;

    //Set SVM highside to new level and check if a VCore increase is possible
    SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * level);

    //Wait until SVM highside is settled
    while ((PMMIFG & SVSMHDLYIFG) == 0) ;
    PMMIFG &= ~SVSMHDLYIFG;

    //Check if a VCore increase is possible
    if (PMMIFG & SVMHIFG == SVMHIFG){
        // Vcc is too low for a Vcore increase
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

    //Vcc is high enough for a Vcore increase
    //Set also SVS highside to new level
    SVSMHCTL |= (SVSHRVL0 * level);

    //Wait until SVM highside is settled
    while (PMMIFG & SVSMHDLYIFG == 0) ;
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

// Ramps up Vcore voltage to desired level.
uint8_t setVCore (uint8_t level){
    uint8_t actlevel;
    uint8_t status = 0;
    uint16_t interruptState;

    //Set Mask for Max. level
    level &= PMMCOREV_3;

    //Get actual VCore
    actlevel = (PMMCTL0 & PMMCOREV_3);

    //Disable interrupts
    interruptState = __get_interrupt_state();
    __disable_interrupt();
    __no_operation();

    // Ramp voltage up
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

  // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
  do
  {
    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
				   // Clear XT2,XT1,DCO fault flags
    SFRIFG1 &= ~OFIFG;		   // Clear fault flags
  } while (SFRIFG1&OFIFG);	   // Test oscillator fault flag

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

  P4DIR |= BIT7; 	// Output pin.

  while(1){

    volatile unsigned int i;

    i = 10000;
    do i--;
    while(i != 0);

    P4OUT ^= BIT7;

  }
}
