#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include "colors.h"
#include "wait_until.h"
#include "img/ducky16.h"

// times defined in cycles - determine programmatically???
const uint16_t LINE_PERIOD = 1271;
const uint16_t HSYNC_PULSE = 94;
const uint16_t FRONT_PORCH = 30;
const uint16_t BACK_PORCH = 91;
const uint16_t BORDER_WIDTH = 68;
const uint16_t FRONT_PORCH_FUDGE = FRONT_PORCH + 24; // fudged front porch, to make sure we get into the interrupt in time
const uint16_t FIELD_LINES = 262;

const uint16_t BORDER_START = 23;
const uint16_t PICTURE_START = 41;
const uint16_t PICTURE_END = 241;
const uint16_t BORDER_END = 259;

const uint8_t LINES_PER_PIXEL = 5;

const uint8_t X = 40;
const uint8_t Y = 40;

uint8_t color_bg = CYAN;
uint16_t field_line = 0;
uint16_t pixel_line = 0;

volatile uint16_t frame = 0;
volatile uint8_t *screen = (uint8_t *)malloc(X * Y);
volatile uint8_t *screen_line = screen;
volatile uint8_t *screen_pixel = screen;

uint16_t val = 0;

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
                         | TCA_SINGLE_CMP1_bm); // and CMP1 - start of pixel drawing

  sei();

  for (int i = 0; i < (X * Y / 2); i++)
  {
    screen[i] = i;
  }

  bool ducky = false;
  volatile uint16_t local_frame = 0;
  while (true)
  {
    // if (frame - local_frame >= 0xA0)
    // {
    //   if (ducky)
    //   {
    //     for (int i = 0; i < X * Y; i++)
    //     {
    //       screen[i] = i;
    //       // screen[i] = pgm_read_byte(ducky16 + i);
    //     }
    //     ducky = false;
    //   }
    //   else
    //   {
    //     for (int i = 0; i < X * Y; i++)
    //     {
    //       screen[i] = pgm_read_byte(ducky16 + i);
    //     }
    //     ducky = true;
    //   }
    //   local_frame += 0xA0;
    // }
  }

  return 0;
}

ISR(TCA0_CMP0_vect) // TCA0 CPM0 routine - front porch
{
  VPORTA_OUT |= PIN3_bm; // BLANK high
  VPORTC_OUT = 0;

  switch (field_line) // line-specific thingies
  {
  case 1:
    // line 0 - set vsync high
    PORTA.OUT |= PIN1_bm;
    frame++;
    screen[0] = frame;
    screen[1] = frame >> 8;
    break;
  case 4:
    // line 1 - set vsync low
    PORTA.OUT &= ~PIN1_bm;
    break;
  case BORDER_START:
    // border start - enable overflow interrupt
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm;
    break;
  case BORDER_END:
    // border end - disable overflow interrupt
    TCA0.SINGLE.INTCTRL &= ~TCA_SINGLE_OVF_bm;
    break;
  case FIELD_LINES:
    field_line = 0;
    break;
  case PICTURE_START:
    // picture start - enable CMP1 interrupt (pixel-pushing)
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP1_bm;
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm;
    pixel_line = 0;
    screen_line = screen;
    screen_pixel = screen;
    break;
  case PICTURE_END:
    // picture end - disable CPM1 interrupt
    TCA0.SINGLE.INTCTRL &= ~TCA_SINGLE_CMP1_bm;
    break;
  }

  field_line++;
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm; // clear CMP0 interrupt flag
}

ISR(TCA0_CMP1_vect) // TCA0 CPM1 routine - start of drawing period
{

  uint8_t draw_byte;

  for (int i = 0; i < X / 2; i++) // loop to push pixels
  {

    draw_byte = *screen_pixel++;
    VPORTC_OUT = draw_byte;
    VPORTC_OUT = draw_byte >> 4;
  }

  _delay_loop_1(5);
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm; // clear CMP1 interrupt flag
  VPORTC_OUT = color_bg;                     // left frame border - output color 0
  pixel_line++;
  if (pixel_line == LINES_PER_PIXEL)
  {
    pixel_line = 0;
    screen_line += X / 2;
  }
  screen_pixel = screen_line;
}

ISR(TCA0_OVF_vect) // TCA0 overflow routine - each line
{
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // clear OVF interrupt flag

  wait_until(HSYNC_PULSE + BACK_PORCH); // wait until start of active period

  VPORTA_OUT &= ~PIN3_bm; // BLANK low
  VPORTC_OUT = color_bg;
}
