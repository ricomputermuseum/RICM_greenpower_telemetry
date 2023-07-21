from micropython import const
import machine
import time
import math

# a class abstraction for a digital hall effect sensor used as a rev counter

class hall():
    
    def __init__(self, pin, diameter, scale):
        """Initialize the hall effect sensor

        Arguments:
        pin      -- pin number that the hall effect sensor is connected to
        diameter -- wheel diameter in centimeters
        scale    -- the scale factor of how many revolutions of the magnet per revolution of the tire
        """

        self.pin = machine.Pin(pin)
        self.diameter_scaled = scale*diameter # Scale the diameter.
        self.ticks = time.ticks_ms() # Take the current ticks (milliseconds).

        # Create an array for the moving average.
        self.readings = [None] * 5

        # Configure the IRQ for the hall effect sensor pin.
        self.pin.irq(trigger=machine.Pin.IRQ_RISING, handler=self.rpm_event)


    def rpm_event(self, pin):
        """Calculate RPM and store it in the array"""
        t = time.ticks_ms() # Get the current ticks (milliseconds).
        period_ms = time.ticks_diff(t, self.ticks) # Calculate the time since last reading.

        # RPM is calculated as 1 minute (60 * 1000ms) divided by the period_ms.
        # For example, if period_ms is 60,000, then RPM is 1.
        rpm = (60*1000)/period_ms

        # Add this reading to the array and pull the first value off the array.
        self.readings = (self.readings + [rpm])[1:]

        print(self.readings)
        self.ticks = time.ticks_ms() # Store the current ticks.


    def get_rpm(self):
        """Get the current RPM reading.

        Returns
        -1 if there are no readings yet
        The rolling average RPM value
        """

        # Don't try to add any None values in the array
        readings = [x for x in self.readings if x is not None]
        if len(readings) > 0:
            return(sum(readings)/len(readings))
        else:
            return -1


    def get_speed_cm_per_minute(self):
        return(self.get_rpm()*self.diameter_scaled*math.pi)


    def get_speed_kph(self):
        km_per_minute = self.get_speed_cm_per_minute()/100000
        return km_per_minute*60


if __name__ == '__main__':
    hall = hall(19, 60, 1)
    while(True):
        print(hall.get_rpm())
        print(f"Speed (cm/min): {hall.get_speed_cm_per_minute()}")
        print(f"Speed (kph): {hall.get_speed_kph()}")
        time.sleep(1)
        #print(machine.Pin(19).value())
