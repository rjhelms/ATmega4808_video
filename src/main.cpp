#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <stdlib.h>
#include <util/delay_basic.h>
#include "colors.h"
#include "wait_until.h"
#include "video_timing.h"
#include "main.h"

#include "img/ducky.h"

#define PICTURE_START 45
#define PICTURE_END 237
#define BORDER_WIDTH 142-45
#define LINES_PER_PIXEL 2

const uint8_t X = 96;
const uint8_t Y = 96;

volatile uint8_t color_bg = CYAN;
uint16_t field_line = 0;
uint16_t pixel_line = 0;

volatile uint16_t frame = 0;
volatile uint8_t *screen = (uint8_t *)malloc(X / 2 * Y);
volatile uint8_t *screen_line;
volatile uint8_t *screen_pixel;

void (*render_line)(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);

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
  TCA0.SINGLE.CMP0 = LINE_PERIOD - (FRONT_PORCH + FUDGE);   // set CMP0 for start of front porch
  TCA0.SINGLE.CMP1 = HSYNC_PULSE + BACK_PORCH + BORDER_WIDTH;
  TCA0.SINGLE.CMP2 = HSYNC_PULSE;               // set CMP2 to hsync pulse
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;     // enable TCA0
  TCA0.SINGLE.INTCTRL = (TCA_SINGLE_OVF_bm      // enable overflow interrupt - start of line timer
                         | TCA_SINGLE_CMP0_bm   // and CMP0 - end of line timer
                         | TCA_SINGLE_CMP1_bm); // and CMP1 - start of pixel drawing
  render_line = &render_line8c;
  sei();

  for (int i = 0; i < (X * Y / 2); i++)
  {
    uint8_t read_byte = pgm_read_byte(img_ducky96 + i);

    screen[i] = read_byte << 4;
    screen[i] += read_byte >> 4;
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
  render_line(X/2, screen_line, color_bg);

  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm; // clear CMP1 interrupt flag
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
  VPORTC_OUT = color_bg;  // left frame border
}

// delay macros from arduino TV out
asm (
	// delay 1 clock cycle.
	".macro delay1\n\t"
		"nop\n"
	".endm\n"
	
	// delay 2 clock cycles
	".macro delay2\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 3 clock cyles
	".macro delay3\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 4 clock cylces
	".macro delay4\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 5 clock cylces
	".macro delay5\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 6 clock cylces
	".macro delay6\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 7 clock cylces
	".macro delay7\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 8 clock cylces
	".macro delay8\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 9 clock cylces
	".macro delay9\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
	
	// delay 10 clock cylces
	".macro delay10\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n"
	".endm\n"
); // end of delay macros

void render_line5c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay2\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line6c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay3\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay1\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line7c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay4\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay2\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line8c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay5\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay3\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line9c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay6\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay4\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line10c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay7\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay5\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line11c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay8\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay6\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line12c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay9\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay7\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line13c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay8\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line14c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay1\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay9\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}