#include <stdlib.h>
#include <mega0_video.h>
#include <colors.h>

#include "img/test_smpte.h"


const uint8_t x_size = 96;
const uint8_t y_size = 96;

Video render;

int main()
{

  // put your setup code here, to run once:
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLA = 0b10000011; // output clock on A7 just for grins
  CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0b00000000;
  
  volatile uint8_t *ptr = (uint8_t *)malloc(x_size / 2 * y_size);
  
  
  for (int i = 0; i < (x_size * y_size / 2); i++)
  {
    uint8_t read_byte = pgm_read_byte(img_test_smpte + i);

    ptr[i] = read_byte << 4;
    ptr[i] += read_byte >> 4;
//    ptr[i] = i;
  }
  
  render = setupVideo(96, 96, ptr, BLACK);
  // bool ducky = false;
  // volatile uint16_t local_frame = 0;
  uint8_t wait_frames = 5;
  while (true)
  {
    if (render.frame > wait_frames)
    {
      render.X += 2;
      render.Y += 2;
      if (render.X == 96)
      {
        wait_frames = 120;
      } else if (render.X > 96)
      {
        wait_frames = 5;
        render.X = 20;
        render.Y = 20;
      }
      setupResolution(render.X, render.Y);
      render.frame = 0;
    }

  }

  return 0;
}

