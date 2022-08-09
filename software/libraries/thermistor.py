from micropython import const
import machine
import time

#a class abstraction for a thermistor

class thermistor():
    
    def __init__(self, pin, const0, const1, const2):
        self.adc = machine.ADC(pin)
        self.const0 = const0
        self.const1 = const1
        self.const2 = const2
        
    def read_temp(self):
        t = self.adc.read_u16()
        t = (((t**const2)*const1)+const0)
        return(t)