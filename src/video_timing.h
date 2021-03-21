/*
    video_timing.h

    Video timing definitions, in microseconds and clock cycles.
*/

#ifndef VIDEO_TIMING_H
#define VIDEO_TIMING_H

#define US_LINE_PERIOD      63.5555
#define US_HSYNC            4.7
#define US_BACK_PORCH       4.5
#define US_FRONT_PORCH      1.7
#define US_ACTIVE_PERIOD    (US_LINE_PERIOD-(US_HSYNC+US_BACK_PORCH+US_FRONT_PORCH))
#define US_TITLE_SAFE_PERIOD (US_ACTIVE_PERIOD * 0.85)
#define US_MID_ACTIVE_PERIOD (US_ACTIVE_PERIOD / 2)

#define FRONT_PORCH_CALL_OFFSET 10 // fudged front porch, to make sure we get into the interrupt in time
#define BORDER_CALL_OFFSET  45  // fudging 

#define CYCLES_PER_US       F_CPU / 1000000
#define LINE_PERIOD         (US_LINE_PERIOD * CYCLES_PER_US )
#define HSYNC_PULSE         (US_HSYNC * CYCLES_PER_US)
#define FRONT_PORCH         (US_FRONT_PORCH * CYCLES_PER_US)
#define BACK_PORCH          (US_BACK_PORCH * CYCLES_PER_US)

#define ACIVE_PERIOD        (US_ACTIVE_PERIOD * CYCLES_PER_US)
#define TITLE_SAFE_PERIOD   (US_TITLE_SAFE_PERIOD * CYCLES_PER_US - BORDER_CALL_OFFSET)
#define MID_ACTIVE_PERIOD   (US_MID_ACTIVE_PERIOD * CYCLES_PER_US)

#define FIELD_LINES         262
#define BORDER_START        16
#define BORDER_END          259
#define MIDDLE_LINE         138
#define MAX_PICTURE_LINES   216 // title safe height



#endif