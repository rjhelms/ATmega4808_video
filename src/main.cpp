#include <stdlib.h>
#include <mega0_video.h>
#include <colors.h>
#include <font5x7.h>

#include "img/test_smpte.h"


const uint8_t x_size = 80;
const uint8_t y_size = 42;

Video *render;

#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// void scroll(volatile uint8_t *screen_ptr, uint8_t lines, uint16_t screen_size)
// {
//   volatile uint8_t *end = screen_ptr + screen_size;
//   volatile uint8_t *offset = screen_ptr + (x_size * lines / 2);
//   asm (
//     "L_%=:\n\t"
//     "ld __tmp_reg__, Y+\n\t"
//     "st X+, __tmp_reg__\n\t"
//     "cp r26, r30\n\t"
//     "cpc r27, r31\n\t"
//     "brne L_%=\n\t"
//     :
//     : "x"(screen_ptr),
//       "y"(offset),
//       "z"(end)
//     : 
//   );
// }

int main()
{
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLA = 0b10000011; // output clock on A7 just for grins
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0b00000000;
  

  PORTA.DIR |= PIN4_bm;
  volatile uint8_t *ptr = (uint8_t *)malloc(x_size / 2 * y_size);
  uint16_t size = (x_size / 2 * y_size);
  for (int i = 0; i < (x_size * y_size / 2); i++)
  {
    // uint8_t read_byte = pgm_read_byte(img_test_smpte + i);

    // ptr[i] = read_byte << 4;
    // ptr[i] += read_byte >> 4;
    ptr[i] = 0;
  }
  
  render = setupVideo(x_size, y_size, ptr, DARK_CYAN);
  PORTMUX_USARTROUTEA = PORTMUX_USART0_ALT1_gc;

  USART0_BAUD = (uint16_t)USART0_BAUD_RATE(9600);
  USART0_CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
  // uint16_t i = 0;
  // uint8_t wait_frames = 5;

  for (int i = 0; i<x_size; i++)
  {
    drawPixel(i, i, CYAN);
    drawPixel(x_size-i-1, i, MAGENTA);
    
  }

  // drawChar(5,0,0x42, font_5x7);
  // drawChar(10,0,0x43, font_5x7);
  uint8_t fg = 0;
  uint8_t bg = 0;
  while (true)
  {
    for (int i = 0x00; i<0x60; i++)
    {
      drawChar((i*5)%80,(i/16)*7,i+0x20, fg, bg, font_5x7);
      fg++;
      if (fg == 16)
      {
        fg = 0;
        bg++;
        bg%= 16;
      }
    }
    bg -= 5;
    bg %= 16;
    delayFrames(10);

    // while (USART0_STATUS & USART_RXCIF_bm)
    // {
    //   // if (i >= size)
    //   // {
    //   //   scroll(ptr, 1, size);
    //   //   i -= (x_size / 2);
    //   // }
    //   uint8_t data = USART0_RXDATAL;
    //   ptr[i] = data << 4;
    //   ptr[i] += data >> 4;
    //   i++;
    //   if (i >= size)
    //     i = 0;
    // }

    // if (render->frame > wait_frames)
    // {
    //   render->X += 2;
    //   render->Y += 2;
    //   if (render->X == 96)
    //   {
    //     wait_frames = 120;
    //   } else if (render->X > 96)
    //   {
    //     wait_frames = 5;
    //     render->X = 20;
    //     render->Y = 20;
    //   }
    //   setupResolution(render->X, render->Y);
    //   render->frame = 0;
    // }
  }

  return 0;
}

