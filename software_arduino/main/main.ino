//The main program for the RICM Greenpower F24 Telemetry Board
//c. 2024 Rhode Island Computer Museum and Emilio Latorre

//---debug---
#define GLOBAL_DEBUG 1 //if set triggers debug messages on serial from all functions
char print_buf[64] = "hello world!"; //buffer for formatted printing using sprintf

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
  isr10mFlag = 1;
}
void isrHandler100m(){ //handler for the 100ms isr
  isr100mFlag = 1;
}
void isrHandler1s(){ //handler for the 1s isr
  isr1sFlag = 1;
}
void isrHandler10s(){ //handler for the 10s isr
  isr10sFlag = 1;
}

//---timestamp---
#include <TimeLib.h>
void resetTime(){
  setTime(0,0,0,1,1,2000); //set time to 0, day/month/year are irrelevant as we are only using to track elapsed time
}

//---sd card---
#include <SPI.h>
#include <SD.h>
#define LOGFILE_NAME "/log.csv"
#define SD_CS_PIN 13
#define SD_DET_PIN 22 //high if a card is physically inserted
File logfile; //the file stream object for writing to the SD card
float data_to_log[16]; //the values of data to be formatted
char log_print_buf[16] = "              "; //buffer for formatted printing to the log file
void setupSD(){ //setup the SPI as connected to the SD card
  pinMode(SD_DET_PIN, INPUT);
  if(digitalRead(SD_DET_PIN)){ //only write if the SD card is inserted
    SPI1.setSCK(10); //using SPI1 on these pins
    SPI1.setTX(11);
    SPI1.setRX(12);
    SD.begin(SD_CS_PIN, SPI1); //the SD library offers hardware CS control. N.B. the SD stream can only be opened once, so if the SD card disconnects at any point a reset will be needed to begin logging again
    Serial.println(F("SD card stream initialized"));
    logfile = SD.open(LOGFILE_NAME, "a");
    logfile.println(F("spd0,spd1,vBat1,vBat2,current,temp0,temp1,temp2,temp3,accX,accY,accZ,gyroX,gyroY,gyroZ,timestamp"));//column names go in the first row
    logfile.close();
  }
  else{ 
    Serial.println(F("No SD card inserted, cannot initialize"));
  }
}
void logToSD(){ //log the raw data in the buffer as formatted CSV data
  if(digitalRead(SD_DET_PIN)){ //only write if the SD card is inserted
    logfile = SD.open(LOGFILE_NAME, "a");
    for(int i = 0; i < 15; i++){ //write out all values as columns in a CSV line
      memset(log_print_buf, 0, 16);
      sprintf(log_print_buf, "%.3f,", data_to_log[i]);
      logfile.print(log_print_buf);
    }
    memset(log_print_buf, 0, 16);
    sprintf(log_print_buf, "\"%02d:%02d:%02d\"\n", hour(), minute(), second());//add timestamp and go to new row
    logfile.print(log_print_buf);
    logfile.close();
  }
  else{ 
    Serial.println(F("No SD card inserted, cannot log data"));
  }
}

//---i2c---
#include <Wire.h>
void setupI2C(){ //setup the I2C buses
  Wire.setSDA(16); //I2C0 goes to the ADC and gyro chips on-board
  Wire.setSCL(17);
  Wire.begin();
  Wire1.setSDA(14); //I2C1 goes to the off-board display header
  Wire1.setSCL(15);
  //Wire1.begin();
}

//---adc---
#include <Adafruit_ADS7830.h>
Adafruit_ADS7830 adc0;
void setupAdc(){ //start the adc with default address on i2c0
  adc0.begin();
}

//---thermistors---
#include "thermistor.h" //the acd-temperature conversion table. As the thermistor resistance equation is roughly quartic and the adc is only 8bits, it is simpler to store the values than calculate them.
void readTemperature(uint8_t channel){ //read the selected channel and store it to the data log buffer
  if(channel > 3){
    Serial.println(F("Invalid thermistor channel selection"));
    return;
  }
  float temp = therm_readings[adc0.readADCsingle(channel+4)];
  data_to_log[(channel+5)] = temp;
  if(GLOBAL_DEBUG){
    memset(print_buf, 0, 64);
    sprintf(print_buf, "channel %d temp: %.1f", channel, temp);
    Serial.println(print_buf);
  }
}

void setup() {
  //---serial---
  Serial.begin(); //begin debug serial
  delay(5000); //debug delay to catch the init messages on usb serial
  //---timers---
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
  //---SD card---
  setupSD();
  //---timestamp---
  resetTime();
  //---I2C---
  setupI2C();
  //---adc---
  setupAdc();
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
    //---thermistors---
    readTemperature(0);
    readTemperature(1);
    readTemperature(2);
  }
  if(isr1sFlag){
    isr1sFlag = 0;
    //---SD card---
    logToSD();
  }
  if(isr10sFlag){
    isr10sFlag = 0;
  }
}
