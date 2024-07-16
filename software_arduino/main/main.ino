//The main program for the RICM Greenpower F24 Telemetry Board
//c. 2024 Rhode Island Computer Museum and Emilio Latorre

//---debug---
#define GLOBAL_DEBUG 1 //if set triggers debug messages on serial from all functions
#define SERIAL_LOGGING 0 //if set log data to serial as well as SD card
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
//declared here instead of in later sections because theya re updated in the timer handlers
uint32_t interval_0, interval_1; //for the hall speed sensors
bool timerHandler(struct repeating_timer *t){ //handler for the hardware timer
  (void) t;
  timer1mFlag = 1; //set appropriate flag
  isr_timers.run(); //update software timers
  interval_0++; //update speed timers, up to 1 minute
  interval_1++;
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
    logfile.println(F("speed,rpm,vBat0,vBat1,current,temp0,temp1,temp2,temp3,accX,accY,accZ,gyroX,gyroY,gyroZ,timestamp"));//column names go in the first row
    logfile.close();
  }
  else{ 
    Serial.println(F("No SD card inserted, cannot initialize"));
  }
}
void logToSD(){ //log the raw data in the buffer as formatted CSV data
  if(digitalRead(SD_DET_PIN)){ //only write if the SD card is inserted
    logfile = SD.open(LOGFILE_NAME, "a");
    if(SERIAL_LOGGING){ //if debugging is on, also print to serial
      Serial.println(F("speed,rpm,vBat1,vBat2,current,temp0,temp1,temp2,temp3,accX,accY,accZ,gyroX,gyroY,gyroZ,timestamp"));//column names go in the first row
    }
    for(int i = 0; i < 15; i++){ //write out all values as columns in a CSV line
      memset(log_print_buf, 0, 16);
      sprintf(log_print_buf, "%.3f,", data_to_log[i]);
      logfile.print(log_print_buf);
      if(SERIAL_LOGGING){
        Serial.print(log_print_buf);
      }
    }
    memset(log_print_buf, 0, 16);
    sprintf(log_print_buf, "\"%02d:%02d:%02d\"\n", hour(), minute(), second());//add timestamp and go to new row
    logfile.print(log_print_buf);
    if(SERIAL_LOGGING){
        Serial.print(log_print_buf);
      }
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
float vbat, vbat1; //voltages in each battery.
void readBatteryVoltages(){ //read battery voltages through a divider and calculate the original voltage
  vbat = (float)(adc0.readADCsingle(0))*0.0516; //0.0129v per level, = 0.1v on the battery through the 68k/10k voltage divider, or 0.0516v on the 30k/10k divider
  vbat1 = ((float)(adc0.readADCsingle(1))*0.1)-vbat; //voltage through both batteries - voltage on the first battery
  data_to_log[2] = vbat;
  data_to_log[3] = vbat1;
  if(GLOBAL_DEBUG){
    memset(print_buf, 0, 64);
    sprintf(print_buf, "vBat0: %.1f, vBat1: %.1f", vbat, vbat1);
    Serial.println(print_buf);
  }
}
float current; //current reading
void readCurrent(){ //read and log the current sensor output value, 0.0258v per step through a 10k/10k divider, with the sensor producng +0.625v per 100a
    current = ((((float)adc0.readADCsingle(2))*0.0258)-(((float)adc0.readADCsingle(3))*0.0258))/0.00625;
    data_to_log[4] = current;
    if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "battery current: %.1f", current);
      Serial.println(print_buf);
    }
}

//---thermistors---
#include "thermistor.h" //the acd-temperature conversion table. As the thermistor resistance equation is roughly quartic and the adc is only 8bits, it is simpler to store the values than calculate them.
void readTemperature(uint8_t channel){ //read the selected channel and store it to the data log buffer N.B. channels will float if a thermistor is not connected
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

//---hall sensors---
//spd_0 is for speed at the wheel, spd_1 is for motor RPM
#include <math.h>
#define WHEEL_DIAMTER_CM 40 //the total diameter of the outside of the tire at inflation pressure
#define INTERVAL_CONST_0 60000 //1rpm interval for spd_0, 60000/magnets per wheel
#define INTERVAL_CONST_1 15000 //1rpm interval for spd_0
float conversion_factor = ((((float)WHEEL_DIAMTER_CM * 3.14) * 60)/100000); //the total linear distance traveled by the tire in a minute converted to km, multiplied by RPM to get kilometers traveled per hour 
float speed = 0; //the vehicle speed in KPH
uint32_t intervals_0[] = {60001,60001,60001,60001,60001}; //median of 5 samples for a less noisy speed reading
uint32_t intervals_1[] = {60001,60001,60001,60001,60001}; 
uint8_t interval_index_0, interval_index_1; //indices for the interval buffers
float rpm0, rpm1;
void updateSpeedInterval0(){ //record the interval between the last and current activations of the sensor
  intervals_0[interval_index_0] = interval_0;
  interval_index_0 = (interval_index_0 + 1) % 5;
  interval_0 = 0;
}
void updateSpeedInterval1(){ //same but for the second sensor
  intervals_1[interval_index_1] = interval_1;
  interval_index_1 = (interval_index_1 + 1) % 5;
  interval_1 = 0;
}
void intervalToRPM(){
  if(interval_0 > INTERVAL_CONST_0){ //wheel is turning at less than 1rpm
    rpm0 = 0;
  }
  else{
    uint64_t sum0 = 0;
    for(uint8_t i = 0; i < 5; i++){ //find the arithmetic mean of the five samples
      sum0 += intervals_0[i];
    }
    uint32_t avg_interval_0 = sum0/(uint32_t)5;
    rpm0 = INTERVAL_CONST_0 / (float)avg_interval_0; //rpm = 1min/magnets per wheel/period
    speed = conversion_factor * rpm0;
  }
  data_to_log[0] = speed; //log wheel speed
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Speed: %.1f", speed);
      Serial.println(print_buf);
  }
  if(interval_1 > INTERVAL_CONST_1){ //wheel is turning at less than 1rpm
    rpm1 = 0;
  }
  else{
    uint64_t sum1 = 0;
    for(uint8_t i = 0; i < 5; i++){
      sum1 += intervals_1[i];
    }
    uint32_t avg_interval_1 = sum1/(uint32_t)5;
    rpm1 = (float)INTERVAL_CONST_1 / (float)avg_interval_1;
  }
  data_to_log[1] = rpm1; //log engine RPM
  if(GLOBAL_DEBUG){
    memset(print_buf, 0, 64);
    sprintf(print_buf, "Motor RPM: %.1f", rpm1);
    Serial.println(print_buf);
  }
}
void setupHall(){
  pinMode(20, INPUT); //SPD_0
  pinMode(19, INPUT); //SPD_1
  attachInterrupt(20, updateSpeedInterval0, FALLING); //update interval whenever the interrupt is triggered
  attachInterrupt(19, updateSpeedInterval1, FALLING); 
}

//---gyro/accelerometer---
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
Adafruit_MPU6050 gyro;
sensors_event_t a, g, t;
void setupGyro(){ //setup the MPU6050
  gyro.begin();
  gyro.setAccelerometerRange(MPU6050_RANGE_4_G); //not likely to accelerate faster than this
  gyro.setGyroRange(MPU6050_RANGE_500_DEG); //ditto angular velocity
  gyro.setFilterBandwidth(MPU6050_BAND_44_HZ); //low pass filter, ignores oscillations faster than this
  gyro.setCycleRate(MPU6050_CYCLE_20_HZ); //set on-chip sampling rate
}
void logGyro(){ //read all sensors and log data
  gyro.getEvent(&a, &g, &t); //acceleration in m/s^2, gyro in */sec, temp in *c
  data_to_log[9] = a.acceleration.x;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Accel x: %.1f", data_to_log[9]);
      Serial.println(print_buf);
  }
  data_to_log[10] = a.acceleration.y;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Accel y: %.1f", data_to_log[10]);
      Serial.println(print_buf);
  }
  data_to_log[11] = a.acceleration.z;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Accel z: %.1f", data_to_log[11]);
      Serial.println(print_buf);
  }
  data_to_log[12] = g.gyro.x;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Gyro x: %.1f", data_to_log[12]);
      Serial.println(print_buf);
  }
  data_to_log[13] = g.gyro.y;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Gyro y: %.1f", data_to_log[13]);
      Serial.println(print_buf);
  }
  data_to_log[14] = g.gyro.z;
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "Gyro z: %.1f", data_to_log[14]);
      Serial.println(print_buf);
  }
  if(GLOBAL_DEBUG){
      memset(print_buf, 0, 64);
      sprintf(print_buf, "MPU chip temp: %.1f", t.temperature);
      Serial.println(print_buf);
  }
}

void setup() {
  //---serial---
  Serial.begin(); //begin debug serial
  if(GLOBAL_DEBUG){
    delay(5000); //debug delay to catch the init messages on usb serial
  }
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
  //---hall sensors---
  setupHall();
  //---gyro/accel---
  setupGyro();
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
    //---hall sensors---
    intervalToRPM();
    //---adc---
    readBatteryVoltages();
    readCurrent();
    //---gyro/accel---
    logGyro();
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
