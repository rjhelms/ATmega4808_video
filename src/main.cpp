#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h> 
#include "colors.h"

// times defined in cycles
const uint16_t LINE_PERIOD = 1271;
const uint8_t HSYNC_PULSE = 94;
const uint8_t FRONT_PORCH = 37;
const uint8_t BACK_PORCH = 85;
const uint8_t BORDER_WIDTH = 208;
const uint8_t FRONT_PORCH_FUDGE = FRONT_PORCH + BORDER_WIDTH + 1;  // fudged front porch, to make sure we get into the interrupt in time
uint16_t palette = (YELLOW << 12 | GREEN << 8 | RED << 4 | DARK_MAGENTA << 0);

int main() {
  // put your setup code here, to run once:
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLA = 0b10000000; // output clock on A7 just for grins
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0b00000000;
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;            // output TCA0 on PORTA
  PORTA_DIR |= (PIN3_bm | PIN2_bm | PIN1_bm);           // set PORTA outputs: PA1 VSYNC, PA2 HSYNC, PA3 BLANK
  PORTC_DIR |= (PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm); // set PORTC outputs: PC0-3 RGBI

  TCA0.SINGLE.CTRLA = 0;                                // disable TCA0 for setup
  TCA0.SINGLE.CTRLD = 0;                                // set single mode
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm             // enable CMP2
    | TCA_SINGLE_WGMODE_SINGLESLOPE_gc);                  // and single-slope mode
  TCA0.SINGLE.PER = LINE_PERIOD;                        // set period
  TCA0.SINGLE.CMP0 = LINE_PERIOD - FRONT_PORCH_FUDGE;   // set CMP0 for start of front porch
  TCA0.SINGLE.CMP2 = HSYNC_PULSE;                       // set CMP2 to hsync pulse
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;             // enable TCA0
  TCA0.SINGLE.INTCTRL = (TCA_SINGLE_OVF_bm              // enable overflow interrupt - start of line timer
    | TCA_SINGLE_CMP0_bm);                                // and CMP0 - end of line timer

  TCB2.INTCTRL &= ~TCB_CAPT_bm;                         /* clobber millis - disable TCB2 interrupt
                                                           just in case this was source of jitter!*/
  sei();

  while (true)
  {
    // main loop - if it exists - goes here
  }
  
}

// wait_until from TVout library - TODO: understand this!
static void inline wait_until(uint8_t time) {
	__asm__ __volatile__ (
			"subi	%[time], 10\n"              // subtract 10 from time (is this how long it takes to get into the function call?)
			"sub	%[time], %[tcnt1l]\n\t"     // subtract counter from time
		"100:\n\t"                        // wait loop
			"subi	%[time], 3\n\t"             // subtract 3 from time - 1 cycle
			"brcc	100b\n\t"                   // if carry cleared, time > 3 - back to loop - 1 cycle if false, 2 if true (this is where 3 comes from!)
			"subi	%[time], 0-3\n\t"           // add 3 back to time - value of time is what it was 2 cycles ago(?)
			"breq	101f\n\t"                   // if time is 0, jump to 101 - 1 cycle if false, 2 if true (3 cycles to end)
			"dec	%[time]\n\t"                // decrement time - 1 cycle
			"breq	102f\n\t"                   // if time is 0, jump to 102 (done) - 2 cycles
			"rjmp	102f\n"                     // otherwise, time was 1 - jump to 102 (done) - 2 cycles
		"101:\n\t"
			"nop\n" 
		"102:\n"
		:
		: [time] "a" (time),              // put time in a register - I bet r16
		[tcnt1l] "a" (TCA0.SINGLE.CNTL)   // put counter in a register
	);
}

ISR(TCA0_CMP0_vect)
{
  PORTC_OUT = palette;
  wait_until(LINE_PERIOD-FRONT_PORCH);
  PORTC_OUT = 0x00;
  PORTA_OUTSET = PIN3_bm;
    
  TCA0.SINGLE.INTFLAGS=TCA_SINGLE_CMP0_bm;
}

ISR(TCA0_OVF_vect)  // TCA0 overflow routine - each line
{
  TCA0.SINGLE.INTFLAGS=TCA_SINGLE_OVF_bm;
  wait_until(HSYNC_PULSE + BACK_PORCH); // wait until start of active period
  PORTA_OUTCLR = PIN3_bm;
  PORTC_OUT = palette;
  wait_until(HSYNC_PULSE + BACK_PORCH + BORDER_WIDTH);
  for (int i = 0; i<80;i++)
  {
    PORTC_OUT = i;
    _NOP();
    _NOP();
  }
  PORTC_OUT = palette;
}

