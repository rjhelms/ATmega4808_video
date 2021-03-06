#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "wait_until.h"
#include "video_timing.h"
#include "mega0_video.h"

Video video;

void (*render_line)(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);

Video* setupResolution(uint8_t x_size, uint8_t y_size)
{
  video.X = x_size;
  video.Y = y_size;
  video.scale = MAX_PICTURE_LINES / y_size;
  video.picture_start = MIDDLE_LINE - ((y_size * video.scale) / 2);
  video.picture_end = video.picture_start + (y_size * video.scale);

  uint8_t cycles_per_pixel = (TITLE_SAFE_PERIOD / x_size);
  if (cycles_per_pixel < 5)
    cycles_per_pixel = 5; // should fail here
  if (cycles_per_pixel > 32)
  {
    cycles_per_pixel = 32;
  }
  else if (cycles_per_pixel > 16)
  {
    // above 16, round by 4 so there's fewer methods for low resolutions
    cycles_per_pixel = cycles_per_pixel / 4;
    cycles_per_pixel = cycles_per_pixel * 4;
  }

  switch (cycles_per_pixel)
  {
    case 5:
      render_line = &render_line5c;
      break;
    case 6:
      render_line = &render_line6c;
      break;
    case 7:
      render_line = &render_line7c;
      break;
    case 8:
      render_line = &render_line8c;
      break;
    case 9:
      render_line = &render_line9c;
      break;
    case 10:
      render_line = &render_line10c;
      break;
    case 11:
      render_line = &render_line11c;
      break;
    case 12:
      render_line = &render_line12c;
      break;
    case 13:
      render_line = &render_line13c;
      break;
    case 14:
      render_line = &render_line14c;
      break;
    case 15:
      render_line = &render_line15c;
      break;
    case 16:
      render_line = &render_line16c;
      break;
    case 20:
      render_line = &render_line20c;
      break;
    case 24:
      render_line = &render_line24c;
      break;
    case 28:
      render_line = &render_line28c;
      break;
    case 32:
      render_line = &render_line32c;
      break;
  }

  video.border_width = MID_ACTIVE_PERIOD - ((x_size * cycles_per_pixel) / 2) - BORDER_CALL_OFFSET;
  TCA0.SINGLE.CMP1 = HSYNC_PULSE + BACK_PORCH + video.border_width;
  return &video;
}

Video* setupVideo(uint8_t x_size, uint8_t y_size, volatile uint8_t *ptr, uint8_t background)
{
  video.screen = ptr;
  video.screen_line = ptr;
  video.color_bg = background;
  
  video.frame = 0;
  video.field_line = 0;
  video.pixel_line = 0;
  
  video.hbi = false;
  video.vbi = false;
  
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;            // output TCA0 on PORTA
  PORTA_DIR |= (PIN3_bm | PIN2_bm | PIN1_bm);           // set PORTA outputs: PA1 VSYNC, PA2 HSYNC, PA3 BLANK
  PORTC_DIR |= (PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm); // set PORTC outputs: PC0-3 RGBI

  TCA0.SINGLE.CTRLA = 0;                                    // disable TCA0 for setup
  TCA0.SINGLE.CTRLD = 0;                                    // set single mode
  TCA0.SINGLE.CTRLB = (TCA_SINGLE_CMP2EN_bm                 // enable CMP2
                       | TCA_SINGLE_WGMODE_SINGLESLOPE_gc); // and single-slope mode
  TCA0.SINGLE.PER = LINE_PERIOD;                            // set period
  TCA0.SINGLE.CMP0 = LINE_PERIOD - (FRONT_PORCH + FRONT_PORCH_CALL_OFFSET);   // set CMP0 for start of front porch
  TCA0.SINGLE.CMP2 = HSYNC_PULSE;               // set CMP2 to hsync pulse
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;     // enable TCA0
  TCA0.SINGLE.INTCTRL = (TCA_SINGLE_OVF_bm      // enable overflow interrupt - start of line timer
                         | TCA_SINGLE_CMP0_bm   // and CMP0 - end of line timer
                         | TCA_SINGLE_CMP1_bm); // and CMP1 - start of pixel drawing

  setupResolution(x_size, y_size);

  sei();
  return &video;
}

void setByte(uint8_t x, uint8_t y, uint8_t value)
{
  volatile uint8_t *byte_ptr = video.screen;
  byte_ptr += x;
  byte_ptr += y * (video.X / 2);
  *byte_ptr = value;
}

void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  volatile uint8_t byte = (color << 4) + color;
  x = x / 2;
  w = w / 2;
  for (int j=0; j<h; j++)
  {
    for (int i=0; i<w; i++)
    {
      video.screen[x+i + (y+j)*(video.X/2)] = byte;
    }
  }
}

void drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
  volatile uint8_t *pixel_ptr = video.screen;
  pixel_ptr += x >> 1;
  pixel_ptr += y * (video.X / 2);
  uint8_t byte = *pixel_ptr;
  if (x & 1)
  {
    byte = byte & 0x0F;
    byte += color << 4;
  } else 
  {
    byte = byte & 0xF0;
    byte += color;
  }
  *pixel_ptr = byte;
}

void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
  int8_t delta_x = x2 - x1;
  int8_t delta_y = y2 - y1;
  float step_x;
  float step_y;
  uint8_t steps = 0;

  if (abs(delta_x) > abs(delta_y))
  {
    if (delta_x < 0)
    {
      step_x = -1;
      step_y = (float)delta_y / -delta_x;
    }
    else {
      step_x = 1;
      step_y = (float)delta_y / delta_x;
    }
    steps = abs(delta_x);
  } else {
    if (delta_y < 0)
    {
      step_y = -1;
      step_x = (float)delta_x / -delta_y;
    }
    else {
      step_y = 1;
      step_x = (float)delta_x / delta_y;
    }
    steps = abs(delta_y);
  }

  float x = x1;
  float y = y1;
  while (steps > 0)
  {
    drawPixel(x, y, color);
    x += step_x;
    y += step_y;
    steps--;
  }
}

void drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  // clamp range
  while (c > pgm_read_byte(f+3))
    c -= 0x20;
  while (c < pgm_read_byte(f+2))
    c += 0x20;
  c -= pgm_read_byte(f+2);
  uint16_t offset = (c * pgm_read_byte(f+1)) + 4;
  for (int j=0; j<pgm_read_byte(f+1); j++)
  {
    uint8_t byte = pgm_read_byte(f+offset);
    for (int i=pgm_read_byte(f)-1; i>=0; i--)
    {
      if (byte & 1<<i)
      {
        drawPixel(x,y,fg);
      } else {
        drawPixel(x,y,bg);
      }
      x++;
    }
    offset++;
    y++;
    x -= pgm_read_byte(f);
  }
}

void drawChar(uint8_t x, uint8_t y, unsigned char c, const unsigned char *f)
{
  drawChar(x, y, c, 0xF, 0x0, f);
}

uint8_t drawUnsignedInt(uint8_t x, uint8_t y, uint32_t val, uint8_t base, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  unsigned char buf[8 * sizeof(uint32_t)];
  unsigned char temp;
  unsigned long i = 0;
  if (val == 0)
  {
    drawChar(x, y, '0', fg, bg, f);
  }
  while (val > 0)
  {
    buf[i] = val % base;
    if (buf[i] >= 10)
    {
      buf[i] += 7; // skip from 9 (ASCII 0x39) to A (ASCII 0x41)
    }
    val /= base;
    i++;
  }
  
  uint8_t loc = 0;
  for (; i > 0; i--)
  {
    drawChar(loc*f[0] + x, y, '0' + buf[i-1], fg, bg, f);
    loc++;
  }
  return loc;
}

uint8_t drawUnsignedInt(uint8_t x, uint8_t y, uint32_t val, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  return drawUnsignedInt(x, y, val, 10, fg, bg, f);
}

uint8_t drawSignedInt(uint8_t x, uint8_t y, int32_t val, uint8_t base, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  uint8_t loc = 0;
  if (val < 0)
  {
    drawChar(x, y, '-', fg, bg, f);
    val = -val;
    loc++;
  }
  loc += drawUnsignedInt(x+loc*f[0], y, val, base, fg, bg, f);
  return loc;
}

uint8_t drawSignedInt(uint8_t x, uint8_t y, int32_t val, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  return drawSignedInt(x, y, val, 10, fg, bg, f);
}

uint8_t drawFloat(uint8_t x, uint8_t y, float val, uint8_t base, uint8_t precision, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  uint8_t loc = drawSignedInt(x, y, (int32_t)val, base, fg, bg, f);
  if (val < 0)
    val = -val;
  drawChar(x + loc++*f[0], y, '.', fg, bg, f);
  for (int i = 0; i < precision; i++)
  {
    val -= (int)val;
    val *= base;
    drawUnsignedInt(x + loc++*f[0], y, (uint32_t) val, base, fg, bg, f);
  }
  return loc;
}

uint8_t drawFloat(uint8_t x, uint8_t y, float val, uint8_t precision, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  return drawFloat(x, y, val, 10, precision, fg, bg, f);
}

uint8_t drawStr(uint8_t x, uint8_t y, const char *str, uint8_t fg, uint8_t bg, const unsigned char *f)
{
  uint8_t loc = 0;
  while (*str)
    drawChar(x + loc++*f[0], y, *str++, fg, bg, f);
  return loc;
}

void delayFrames(uint16_t frames)
{
  uint16_t start_frame = video.frame;
  while (video.frame - start_frame < frames)
  {;}
}


ISR(TCA0_CMP0_vect) // TCA0 CPM0 routine - front porch
{

  video.hbi = true;
  VPORTA_OUT |= PIN3_bm; // BLANK high
  VPORTC_OUT = 0;

  switch (video.field_line) // line-specific thingies
  {
  case 1:
    // line 0 - set vsync high
    PORTA.OUT |= PIN1_bm;
    video.frame++;
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
    video.vbi = true;
    break;
  case FIELD_LINES:
    video.field_line = 0;
    break;
  default:
    if (video.field_line == video.picture_start)
    {
      // picture start - enable CMP1 interrupt (pixel-pushing)
      TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP1_bm;
      TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm;
      video.pixel_line = 0;
      video.screen_line = video.screen;
      break;
    }
    if (video.field_line == video.picture_end)
    {
      // picture end - disable CPM1 interrupt
      TCA0.SINGLE.INTCTRL &= ~TCA_SINGLE_CMP1_bm;
      break;
    }
  }

  video.field_line++;
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm; // clear CMP0 interrupt flag
}

ISR(TCA0_CMP1_vect) // TCA0 CPM1 routine - start of drawing period
{
  render_line(video.X/2, video.screen_line, video.color_bg);

  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm; // clear CMP1 interrupt flag
  video.pixel_line++;
  if (video.pixel_line == video.scale)
  {
    video.pixel_line = 0;
    video.screen_line += video.X / 2;
  }
}

ISR(TCA0_OVF_vect) // TCA0 overflow routine - each line
{
  wait_until(HSYNC_PULSE + BACK_PORCH); // wait until start of active period
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // clear OVF interrupt flag
  
  VPORTA_OUT &= ~PIN3_bm; // BLANK low
  VPORTC_OUT = video.color_bg;  // left frame border
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

void render_line15c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay2\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
    "brne L_%=\n\t"               // loop until pixel counter is 0 - 2 cyc
    "delay3\n\t"
    "out 0x9, %[color_bg]\n\t"
    :
    : [counter] "a"(x),
      [ptr] "z"(ptr),
      [color_bg] "a"(color_bg)
  );
}

void render_line16c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay3\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
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

void render_line20c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay7\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
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

void render_line24c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay10\n\t"
    "delay1\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
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

void render_line28c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay10\n\t"
    "delay5\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
    "delay10\n\t"
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

void render_line32c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg)
{
  asm volatile (
    "L_%=:\n\t"
    "ld __tmp_reg__,Z+\n\t"       // load the value at screen pointer into r24 & post-increment - 2 cyc
    "out 0x9, __tmp_reg__\n\t"    // push the byte at r24 to PORTC - low nibble - 1 cyc
    "swap __tmp_reg__\n\t"        // swap the two nibbles - 1 cyc
    "dec %[counter]\n\t"          // decrement the pixel counter - 1 cyc
    "delay10\n\t"
    "delay10\n\t"
    "delay9\n\t"
    "out 0x9, __tmp_reg__\n\t"    // push the byte at 24 to PORTC again - 1 cyc
    "delay10\n\t"
    "delay10\n\t"
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
