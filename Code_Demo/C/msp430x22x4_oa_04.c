//******************************************************************************
//  MSP430F22x4 Demo - OA0, Non-Inverting PGA Mode
//
//  Description: Configure OA0 for Non-Inverting PGA mode. In this mode,
//  the "-" terminal is connected to the R ladder tap and the OAFBRx bits
//  select the gain.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430F22x4
//             -------------------
//         /|\|                XIN|-
//          | |                   |
//          --|RST            XOUT|-
//            |                   |
//     "+" -->|P2.0/A0/OA0I0      |
//            |                   |
//            |       P2.1/A1/OA0O|--> OA0 Output
//            |                   |    Gain is 8
//
//  A. Dannenberg
//  Texas Instruments Inc.
//  March 2006
//  Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.40B
//******************************************************************************
#include "msp430x22x4.h"

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  OA0CTL0 = OAPM_1 + OAADC1;                // "+" connected to OA0IO (default),
                                            // Slow slew rate,
                                            // Output connected to A1/OA0O
  OA0CTL1 = OAFBR_6 + OAFC_4;               // Gain is 8,
                                            // Non-inverting PGA mode
  ADC10AE0 = 0x03;                          // P2.1/0 analog function select
  LPM3;                                     // Enter LPM3
}
