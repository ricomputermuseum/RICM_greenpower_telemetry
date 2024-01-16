from micropython import const
import machine
import time

last_tick = time.ticks_us()
last_interval = 1;

def hall_handler(pin): #handler to update speed sensor interval when hall sensor is tripped
    t = time.ticks_us()
    global last_interval = time.ticks_diff(t,last_tick)
    global last_tick = t
    

#a class abstraction for a digital hall effect sensor used as a rev counter

class hall():
    
    def __init__(self, pin, diameter):
        self.pin = machine.Pin(pin, machine.Pin.IN)
        self.scale = 5280/(6.28*(diameter/12)) #number of revolutions per mile given wheel diameter in inches
        self.pin.irq(trigger=Pin.IRQ_RISING, handler=hall_handler)
        
    def get_rpm(self):
        return((1000000/last_interval)*60)
    
    def get_speed(self):
        return((scale/self.get_rpm())*60)
    
    def get_speed_kph(self):
        return(self.get_speed()*1.6)
    
def hall_1_debug():
    hall1 = hall(19,12)
    for i in range(1000):
        print(hall1.get_speed)
        time.sleep_ms(1000)    