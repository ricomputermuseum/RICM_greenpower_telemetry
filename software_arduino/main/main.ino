//The main program for the RICM Greenpower F24 Telemetry Board
//c. 2024 Rhode Island Computer Museum and Emilio Latorre

//---debug---
#define GLOBAL_DEBUG 1 //if set triggers general debug messages on the serial port. generally want to avoid

//---timers---
#define TIMER_INTERRUPT_DEBUG       0
#define _TIMERINTERRUPT_LOGLEVEL_   0
#include <RPi_Pico_TimerInterrupt.h>
#include <RPi_Pico_ISR_Timer.h>
RPI_PICO_Timer itimer1(1); //the hardware timer
RPI_PICO_ISR_Timer isr_timers; //the software timers
#define HW_TIMER_INTERVAL_US   1000L  //run hardware timer on a 1ms period, for scheduling critical tasks
#define TIMER_INTERVAL_10M     10L    //software interrupt timers with periods of 10ms, 100ms, 1s, 10s
#define TIMER_INTERVAL_100M    100L   //we can have up to 16 of these but the fewer the faster it will run
#define TIMER_INTERVAL_1S      1000L  //although these are not hardware interrupts, the handlers still need to be light 
#define TIMER_INTERVAL_10S     10000L //(absolutely no serial prints, ideally just setting flags)
uint8_t timer1mFlag, isr10mFlag, isr100mFlag, isr1sFlag, isr10sFlag; //flags for scheduling tasks with the timers

//---timer handlers---
bool timerHandler(struct repeating_timer *t){ //handler for the hardware timer
  (void) t;
  timer1mFlag = 1; //set appropriate flag
  isr_timers.run(); //update software timers
  return true;
}
void isrHandler10m(){ //handler for the 10ms isr
  if(GLOBAL_DEBUG){
    Serial.println(F("10ms Tasks Running"));
  }
  isr10mFlag = 1;
}
void isrHandler100m(){ //handler for the 100ms isr
  if(GLOBAL_DEBUG){
    Serial.println(F("100ms Tasks Running"));
  }
  isr100mFlag = 1;
}
void isrHandler1s(){ //handler for the 1s isr
  if(GLOBAL_DEBUG){
    Serial.println(F("1s Tasks Running"));
  }
  isr1sFlag = 1;
}
void isrHandler10s(){ //handler for the 10s isr
  if(GLOBAL_DEBUG){
    Serial.println(F("10s Tasks Running"));
  }
  isr10sFlag = 1;
}



void setup() {
  Serial.begin(); //begin debug serial
  if(itimer1.attachInterruptInterval(HW_TIMER_INTERVAL_US, timerHandler)){ //setup the timer with handlers
    Serial.println(F("Main timer running with period 1000us, setting up software timers"));
    isr_timers.setInterval(TIMER_INTERVAL_10M, isrHandler10m); //set software timers with appropriate intervals and handleers
    isr_timers.setInterval(TIMER_INTERVAL_100M, isrHandler100m);
    isr_timers.setInterval(TIMER_INTERVAL_1S, isrHandler1s);
    isr_timers.setInterval(TIMER_INTERVAL_10S, isrHandler10s);
  }
  else{
    Serial.println(F("Failed to start main timer. Software timers will not update"));
  }
}

void loop() {
  if(timer1mFlag){
    timer1mFlag = 0;
  }
  if(isr10mFlag){
    isr10mFlag = 0;
  }
  if(isr100mFlag){
    isr100mFlag = 0;
  }
  if(isr1sFlag){
    isr1sFlag = 0;
  }
  if(isr10sFlag){
    isr10sFlag = 0;
  }
}
