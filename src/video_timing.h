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
#define US_ACTIVE_PERIOD    US_PER_LINE-(US_HYNC+US_BACK_PORCH+US_FRONT_PORCH)

#define CYCLES_PER_US       F_CPU / 1000000
#define LINE_PERIOD         (US_LINE_PERIOD * CYCLES_PER_US )
#define HSYNC_PULSE         (US_HSYNC * CYCLES_PER_US)
#define FRONT_PORCH         (US_FRONT_PORCH * CYCLES_PER_US)
#define BACK_PORCH          (US_BACK_PORCH * CYCLES_PER_US)
#define FUDGE               10 // fudged front porch, to make sure we get into the interrupt in time

#define FIELD_LINES     262
#define BORDER_START    16
#define BORDER_END      259

#endif