from micropython import const
import machine
import time

#a class abstraction for a digital hall effect sensor used as a rev counter

class hall():
    
    def __init__(self, pin, diameter, scale):
        self.pin = machine.Pin(pin)
        self.scale = scale*diameter #the scale factor of how many revolutions of the magnet per revolution of the tire
        self.ticks = time.ticks_us();
        self.period_us = 0
        
    def update_period(self):
        t = time.ticks_us()
        period_us = time.ticks_diff(self.ticks, t)
        self.ticks = t()
        
    def get_rpm(self):
        