#ifndef TimerThree_h
#define TimerThree_h

#include "Arduino.h"

#define TIMER_RESOLUTION 65536UL  // Timer is 16 bit

class TimerThree
{
  public:
    //****************************
    //  Configuration
    //****************************
    void initialize(uint32_t microseconds = 1000000) __attribute__((always_inline)) {
      TCCR3B = _BV(WGM33);
      TCCR3A = 0;
      setPeriod(microseconds);
    }

    void setPeriod(uint32_t microseconds) __attribute__((always_inline)) {
      const uint32_t cycles = ((F_CPU / 100000 * microseconds) / 20);

      if (cycles < TIMER_RESOLUTION) {
        clockSelectBits = _BV(CS30);
        pwmPeriod = cycles;
      } else if (cycles < TIMER_RESOLUTION * 8) {
        clockSelectBits = _BV(CS31);
        pwmPeriod = cycles / 8;
      } else if (cycles < TIMER_RESOLUTION * 64) {
        clockSelectBits = _BV(CS31) | _BV(CS30);
        pwmPeriod = cycles / 64;
      } else if (cycles < TIMER_RESOLUTION * 256) {
        clockSelectBits = _BV(CS32);
        pwmPeriod = cycles / 256;
      } else if (cycles < TIMER_RESOLUTION * 1024) {
        clockSelectBits = _BV(CS32) | _BV(CS30);
        pwmPeriod = cycles / 1024;
      } else {
        clockSelectBits = _BV(CS32) | _BV(CS30);
        pwmPeriod = TIMER_RESOLUTION - 1;
      }

      ICR3 = pwmPeriod;
      TCCR3B = _BV(WGM33) | clockSelectBits;
    }

    //****************************
    //  Run Control
    //****************************
    void start() __attribute__((always_inline)) {
      TCCR3B = 0;
      TCNT3 = 0;		// TODO: does this cause an undesired interrupt?
      resume();
    }

    void stop() __attribute__((always_inline)) {
      TCCR3B = _BV(WGM33);
    }

    void restart() __attribute__((always_inline)) {
      start();
    }

    void resume() __attribute__((always_inline)) {
      TCCR3B = _BV(WGM33) | clockSelectBits;
    }

    //****************************
    //  Interrupt Function
    //****************************
    void attachInterrupt(void (*isr)()) __attribute__((always_inline)) {
      isrCallback = isr;
      TIMSK3 = _BV(TOIE1);
    }

    void detachInterrupt() __attribute__((always_inline)) {
      TIMSK3 = 0;
    }

    static void (*isrCallback)();
    static void isrDefaultUnused();

  private:
    static uint16_t pwmPeriod;
    static uint8_t clockSelectBits;
};

extern TimerThree Timer3;

#endif
