#include "TimerThree.h"

TimerThree Timer3;

uint16_t TimerThree::pwmPeriod = 0;
uint8_t TimerThree::clockSelectBits = 0;
void (*TimerThree::isrCallback)() = TimerThree::isrDefaultUnused;
void TimerThree::isrDefaultUnused() {
  /* noop */;
}

ISR(TIMER3_OVF_vect) {
  Timer3.isrCallback();
}
