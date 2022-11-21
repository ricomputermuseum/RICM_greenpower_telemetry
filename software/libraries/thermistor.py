import machine
from utime import sleep_ms
import ADC

#a class abstraction for a thermistor, using the ADS7830

class thermistor():
    
    def __init__(self, number = 0, address = 0x48, sda=16, scl=17, bus=0, f=False):
        sda0=machine.Pin(16)
        scl0=machine.Pin(17)
        i2c0=machine.I2C(bus, sda=sda0, scl=scl0, freq=400000)
        self.adc = ADC.ADS7830(i2c0)
        self.number = number+4 #which thermistor is this? (0-3), offset is because of the way the pins are setup on our board
        self.f = f
        self.t_celsius = 0
        self.t_farenheit = 0
    
    #equation that takes the raw voltage level from the thermistor and converts it to a celsius temperature
    def equation(self, raw):
        temp = -54.9 + (1.52*raw) + (-0.011*pow(raw,2)) + (0.0000312*pow(raw,3)) #cubic polynomial is not ideal but we only sample at a low rate so it should be ok
        return(temp) #using this equation, the absolute deviation should not exceed 8 degrees, consider rounding to 5 degrees
    
    def read(self):
        raw = self.adc.read_channel_se(self.number)
        self.t_celsius = int(self.equation(raw))
        self.t_farenheit = int((self.t_celsius*5/9)+32)
    
    def __str__(self):
        self.read()
        if(self.f):
            return(str(self.t_farenheit))
        else:
            return(str(self.t_celsius))
            
    def __repr__(self):
        self.read()
        if(self.f):
            return(str(self.t_farenheit))
        else:
            return(str(self.t_celsius))
        
#debug code
t0 = thermistor()

def debug(n=255):
    for i in range(n):
        print(t0)
        sleep_ms(500)
        
    
        