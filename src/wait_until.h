/*
    wait_until.h 
    cycle-perfect wait based on TCA0.CNTL

    Adapted for megaAVR-0 family from original in arduino-tvout:
    https://github.com/nootropicdesign/arduino-tvout
*/

// my timing counts are wrong

static void inline wait_until(uint8_t time)
{
    __asm__ __volatile__(
        "subi	%[time], 10\n"          // subtract 10 from time (is this how long it takes to get into the function call?)
        "sub	%[time], %[tcnt1l]\n\t" // subtract counter from time
        "100:\n\t"                    // wait loop:
        "subi	%[time], 3\n\t"         // subtract 3 from time - 1 cycle
        "brcc	100b\n\t"               // if carry cleared, time > 3 - back to loop - 1 cycle if false, 2 if true (this is where 3 comes from!)
        "subi	%[time], 0-3\n\t"       // add 3 back to time - value of time is what it was 2 cycles ago(?)
        "breq	101f\n\t"               // if time is 0, jump to 101 - 1 cycle if false, 2 if true (3 cycles to end)
        "dec	%[time]\n\t"            // decrement time - 1 cycle
        "breq	102f\n\t"               // if time is 0, jump to 102 (done) - 2 cycles
        "rjmp	102f\n"                 // otherwise, time was 1 - jump to 102 (done) - 2 cycles
        "101:\n\t"
        "nop\n"
        "102:\n"
        :
        : [time] "a"(time),              // put time in a register - I bet r16
          [tcnt1l] "a"(TCA0.SINGLE.CNTL) // put counter in a register
    );
}
