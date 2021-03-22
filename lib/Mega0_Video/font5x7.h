#include <avr/pgmspace.h>

#ifndef FONT5X7_H
#define FONT5X7_H

PROGMEM const unsigned char font_5x7[] = {
    5,      // width
    7,      // height
    0x20,   // ASCII index of lowest char
    0x7F,   // ASCII index of highest char
    0b00000, // 20 SPACE
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,

    0b00100, // 21 !
    0b00100,
    0b00100,
    0b00100,
    0b00000,
    0b00100,
    0b00000,

    0b01010, // 22 "
    0b01010,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,

    0b00000, // 23 #
    0b01010,
    0b11111,
    0b01010,
    0b11111,
    0b01010,
    0b00000,

    0b00100, // 24 $
    0b11110,
    0b10100,
    0b11110,
    0b01010,
    0b11110,
    0b01000,

    0b00000, // 25 %
    0b10010,
    0b00100,
    0b01000,
    0b10010,
    0b00000,
    0b00000,

    0b00000, // 26 &
    0b01100,
    0b10000,
    0b01101,
    0b10010,
    0b01101,
    0b00000,

    0b01000, // 27 '
    0b01000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,

    0b01100, // 28 (
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b01100,
    0b00000,

    0b01100, // 29 )
    0b00010,
    0b00010,
    0b00010,
    0b00010,
    0b01100,
    0b00000,

    0b00000, // 2A *
    0b10010,
    0b01100,
    0b01100,
    0b10010,
    0b00000,
    0b00000,

    0b00000, // 2B +
    0b00000,
    0b00100,
    0b01110,
    0b00100,
    0b00000,
    0b00000,

    0b00000, // 2C ,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b01000,
    0b10000,

    0b00000, // 2D -
    0b00000,
    0b00000,
    0b01110,
    0b00000,
    0b00000,
    0b00000,

    0b00000, // 2E .
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b01000,
    0b00000,

    0b00000, // 2F /
    0b00010,
    0b00100,
    0b01000,
    0b10000,
    0b00000,
    0b00000,

    0b01100, // 30 0
    0b10010,
    0b10110,
    0b11010,
    0b10010,
    0b01100,
    0b00000,

    0b00100, // 31 1
    0b01100,
    0b00100,
    0b00100,
    0b00100,
    0b01110,
    0b00000,

    0b01100, // 32 2
    0b10010,
    0b00010,
    0b00100,
    0b01000,
    0b11110,
    0b00000,

    0b11100, // 33 3
    0b00010,
    0b11100,
    0b00010,
    0b00010,
    0b11100,
    0b00000,

    0b10010, // 34 4
    0b10010,
    0b11110,
    0b00010,
    0b00010,
    0b00010,
    0b00000,

    0b11110, // 35 5
    0b10000,
    0b01100,
    0b00010,
    0b10010,
    0b01100,
    0b00000,

    0b01100, // 36 6
    0b10000,
    0b11100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b11110, // 37 7
    0b00010,
    0b00100,
    0b00100,
    0b01000,
    0b01000,
    0b00000,

    0b01100, // 38 8
    0b10010,
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b01100, // 39 9
    0b10010,
    0b10010,
    0b01110,
    0b00010,
    0b01100,
    0b00000,

    0b00000, // 3A :
    0b00000,
    0b01000,
    0b00000,
    0b00000,
    0b01000,
    0b00000,

    0b00000, // 3B ;
    0b00000,
    0b01000,
    0b00000,
    0b00000,
    0b01000,
    0b10000,

    0b00010, // 3C <
    0b00100,
    0b01000,
    0b00100,
    0b00010,
    0b00000,
    0b00000,

    0b00000, // 3D =
    0b00000,
    0b11110,
    0b00000,
    0b11110,
    0b00000,
    0b00000,

    0b01000, // 3E >
    0b00100,
    0b00010,
    0b00100,
    0b01000,
    0b00000,
    0b00000,

    0b00100, // 3F ?
    0b01010,
    0b00010,
    0b00100,
    0b00000,
    0b00100,
    0b00000,

    0b01110, // 40 @
    0b10010,
    0b10110,
    0b10110,
    0b10000,
    0b01110,
    0b00000,

    0b01100, // 41 A
    0b10010,
    0b10010,
    0b11110,
    0b10010,
    0b10010,
    0b00000,

    0b11100, // 42 B
    0b10010,
    0b11100,
    0b10010,
    0b10010,
    0b11100,
    0b00000,

    0b01110, // 43 C
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b01110,
    0b00000,

    0b11100, // 44 D
    0b10010,
    0b10010,
    0b10010,
    0b10010,
    0b11100,
    0b00000,

    0b11110, // 45 E
    0b10000,
    0b11100,
    0b10000,
    0b10000,
    0b11110,
    0b00000,

    0b11110, // 46 F
    0b10000,
    0b11100,
    0b10000,
    0b10000,
    0b10000,
    0b00000,

    0b01110, // 47 G
    0b10000,
    0b10110,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b10010, // 48 H
    0b10010,
    0b11110,
    0b10010,
    0b10010,
    0b10010,
    0b00000,

    0b01110, // 49 I
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b01110,
    0b00000,

    0b00110, // 4A J
    0b00010,
    0b00010,
    0b00010,
    0b10010,
    0b01100,
    0b00000,

    0b10010, // 4B K
    0b10100,
    0b11000,
    0b11000,
    0b10100,
    0b10010,
    0b00000,

    0b10000, // 4C L
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b11110,
    0b00000,

    0b10010, // 4D M
    0b11110,
    0b11110,
    0b10010,
    0b10010,
    0b10010,
    0b00000,

    0b10010, // 4E N
    0b10010,
    0b11010,
    0b10110,
    0b10010,
    0b10010,
    0b00000,

    0b01100, // 4F O
    0b10010,
    0b10010,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b11100, // 50 P
    0b10010,
    0b10010,
    0b11100,
    0b10000,
    0b10000,
    0b00000,

    0b01100, // 51 Q
    0b10010,
    0b10010,
    0b10010,
    0b10110,
    0b01110,
    0b00010,

    0b11100, // 52 R
    0b10010,
    0b10010,
    0b11100,
    0b10100,
    0b10010,
    0b00000,

    0b01110, // 53 S
    0b10000,
    0b01000,
    0b00100,
    0b00010,
    0b11100,
    0b00000,

    0b01110, // 54 T
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000,

    0b10010, // 55 U
    0b10010,
    0b10010,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b01010, // 56 V
    0b01010,
    0b01010,
    0b01010,
    0b01010,
    0b00100,
    0b00000,

    0b10010, // 57 W
    0b10010,
    0b10010,
    0b11110,
    0b11110,
    0b10010,
    0b00000,

    0b10010, // 58 X
    0b10010,
    0b01100,
    0b01100,
    0b10010,
    0b10010,
    0b00000,

    0b01010, // 59 Y
    0b01010,
    0b01010,
    0b00100,
    0b00100,
    0b00100,
    0b00000,

    0b11110, // 5A Z
    0b00010,
    0b00100,
    0b01000,
    0b10000,
    0b11110,
    0b00000,

    0b11110, // 5B [
    0b10000,
    0b10000,
    0b10000,
    0b10000,
    0b11110,
    0b00000,

    0b00000, // 5C \'
    0b10000,
    0b01000,
    0b00100,
    0b00010,
    0b00000,
    0b00000,

    0b11110, // 5D ]
    0b00010,
    0b00010,
    0b00010,
    0b00010,
    0b11110,
    0b00000,

    0b01000, // 5E ^
    0b10100,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,

    0b00000, // 5F _
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b11110,
    0b00000,

    0b01000, // 60 `
    0b00100,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,

    0b00000, // 61 a
    0b01100,
    0b00010,
    0b01110,
    0b10010,
    0b01100,
    0b00000,

    0b10000, // 62 b
    0b10000,
    0b11100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b00000, // 63 c
    0b00000,
    0b01110,
    0b10000,
    0b10000,
    0b01110,
    0b00000,

    0b00010, // 64 d
    0b00010,
    0b01110,
    0b10010,
    0b10010,
    0b01110,
    0b00000,

    0b00000, // 65 e
    0b00000,
    0b01100,
    0b11110,
    0b10000,
    0b01110,
    0b00000,

    0b00100, // 66 f
    0b01010,
    0b01000,
    0b11100,
    0b01000,
    0b01000,
    0b00000,

    0b00000, // 67 g
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00010,
    0b01100,

    0b10000, // 68 h
    0b10000,
    0b11100,
    0b10010,
    0b10010,
    0b10010,
    0b00000,

    0b00000, // 69 i
    0b00100,
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b00000,

    0b00000, // 6A j
    0b00100,
    0b00000,
    0b00100,
    0b00100,
    0b10100,
    0b01000,

    0b00000, // 6B k
    0b10000,
    0b10000,
    0b10100,
    0b11000,
    0b10100,
    0b00000,

    0b01000, // 6C l
    0b01000,
    0b01000,
    0b01000,
    0b01000,
    0b01000,
    0b00000,

    0b00000, // 6D m
    0b00000,
    0b11110,
    0b11110,
    0b10010,
    0b10010,
    0b00000,

    0b00000, // 6E n
    0b00000,
    0b01100,
    0b10010,
    0b10010,
    0b10010,
    0b00000,

    0b00000, // 6F 0
    0b00000,
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,

    0b00000, // 70 p
    0b01100,
    0b10010,
    0b10010,
    0b11100,
    0b10000,
    0b10000,

    0b00000, // 71 q
    0b01100,
    0b10010,
    0b10010,
    0b01110,
    0b00010,
    0b00010,

    0b00000, // 72 r
    0b00000,
    0b01100,
    0b10000,
    0b10000,
    0b10000,
    0b00000,

    0b00000, // 73 s
    0b01110,
    0b10000,
    0b01100,
    0b00010,
    0b11100,
    0b00000,

    0b01000, // 74 t
    0b01000,
    0b11100,
    0b01000,
    0b01000,
    0b00100,
    0b00000,

    0b00000, // 75 u
    0b00000,
    0b10010,
    0b10010,
    0b10010,
    0b01110,
    0b00000,

    0b00000, // 76 v
    0b00000,
    0b01010,
    0b01010,
    0b01010,
    0b00100,
    0b00000,

    0b00000, // 77 w
    0b00000,
    0b10010,
    0b10010,
    0b11110,
    0b11110,
    0b00000,

    0b00000, // 78 x
    0b00000,
    0b10010,
    0b01100,
    0b01100,
    0b10010,
    0b00000,

    0b00000, // 79 y
    0b00000,
    0b10010,
    0b10010,
    0b01110,
    0b00010,
    0b01100,

    0b00000, // 7A z
    0b00000,
    0b11110,
    0b00100,
    0b01000,
    0b11110,
    0b00000,

    0b01100, // 7B {
    0b10000,
    0b01000,
    0b01000,
    0b10000,
    0b01100,
    0b00000,

    0b00000, // 7C |
    0b00100,
    0b00100,
    0b00000,
    0b00100,
    0b00100,
    0b00000,

    0b01100, // 7D }
    0b00010,
    0b00100,
    0b00100,
    0b00010,
    0b01100,
    0b00000,

    0b00000, // 7E ~
    0b00000,
    0b01010,
    0b10100,
    0b00000,
    0b00000,
    0b00000,

    0b00000, // 7F DEL
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
};

#endif
