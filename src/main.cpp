#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "colors.h"
#include "wait_until.h"

// times defined in cycles - determine programmatically???
const uint16_t LINE_PERIOD = 1271;
const uint8_t HSYNC_PULSE = 94;
const uint8_t FRONT_PORCH = 30;
const uint8_t BACK_PORCH = 91;
const uint8_t BORDER_WIDTH = 208;
const uint8_t FRONT_PORCH_FUDGE = FRONT_PORCH + 12; // fudged front porch, to make sure we get into the interrupt in time

uint16_t palette = (YELLOW << 12 | GREEN << 8 | RED << 4 | DARK_MAGENTA << 0);

int main()
{
  // put your setup code here, to run once:
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLA = 0b10000000; // output clock on A7 just for grins
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0b00000000;
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;            // output TCA0 on PORTA
  PORTA_DIR |= (PIN3_bm | PIN2_bm | PIN1_bm);           // set PORTA outputs: PA1 VSYNC, PA2 HSYNC, PA3 BLANK
  PORTC_DIR |= (PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm); // set PORTC outputs: PC0-3 RGBI

  TCA0.SINGLE.CTRLA = 0;                                    // disable TCA0 for setup
  TCA0.SINGLE.CTRLD = 0;                                    // set single mode
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm                 // enable CMP2
                       | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // and single-slope mode
  TCA0.SINGLE.PER = LINE_PERIOD;                            // set period
  TCA0.SINGLE.CMP0 = LINE_PERIOD - FRONT_PORCH_FUDGE;       // set CMP0 for start of front porch
  TCA0.SINGLE.CMP1 = HSYNC_PULSE + BACK_PORCH + BORDER_WIDTH;
  TCA0.SINGLE.CMP2 = HSYNC_PULSE;               // set CMP2 to hsync pulse
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;     // enable TCA0
  TCA0.SINGLE.INTCTRL = (TCA_SINGLE_OVF_bm      // enable overflow interrupt - start of line timer
                         | TCA_SINGLE_CMP0_bm   // and CMP0 - end of line timer
                         | TCA_SINGLE_CMP1_bm); // and CMP1 - start of video buffer

  sei();

  while (true)
  {
    // main loop - if it exists - goes here
  }
}


ISR(TCA0_CMP0_vect) // TCA0 CPM0 routine - front porch
{
  VPORTA_OUT |= PIN3_bm; // BLANK high
  VPORTC_OUT = 0x00;     // blanking interval - output black

  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm; // clear CMP0 interrupt flag
}

ISR(TCA0_CMP1_vect) // TCA0 CPM1 routine - start of drawing period
{
  for (int i = 0; i < 80; i++) // loop to push pixels
  {
    VPORTC_OUT = i;
    _NOP();
    _NOP();
    _NOP();
  }
  VPORTC_OUT = palette;                      // right border - output color 0
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm; // clear CMP1 interrupt flag
}

ISR(TCA0_OVF_vect) // TCA0 overflow routine - each line
{
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // clear OVF interrupt flag
  wait_until(HSYNC_PULSE + BACK_PORCH);     // wait until start of active period
  VPORTA_OUT &= ~PIN3_bm;                   // BLANK low
  VPORTC_OUT = palette;                     // left frame border - output color 0
}
