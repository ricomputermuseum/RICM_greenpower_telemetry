from micropython import const
import machine
import utime
import GPIO

sda=machine.Pin(14)
scl=machine.Pin(15)
i2c=machine.I2C(1, sda=sda, scl=scl, freq=400000)

leds = GPIO.PCA9555(i2c)
leds.display_setup()

def debug(n=128):
    for i in range(n) :
        leds.all_low()
        print(leds.display_read())
        utime.sleep_ms(500)
        leds.all_high()
        print(leds.display_read())
        utime.sleep_ms(500)