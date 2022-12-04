# MicroPython SSD1306 OLED driver, I2C and SPI interfaces
# Taken from https://github.com/micropython/micropython/blob/master/drivers/display/ssd1306.py

from micropython import const
import framebuf
import machine
import utime
import OLED

sda=machine.Pin(14)
scl=machine.Pin(15)
i2c=machine.I2C(1, sda=sda, scl=scl, freq=400000)

print('Scan i2c bus...')
devices = i2c.scan()

if len(devices) == 0:
    print("No i2c device !")
else:
    print('i2c devices found:',len(devices))

for device in devices:
    print("Decimal address: ",device," | Hexa address: ",hex(device))
    
    
oled0 = OLED.SSD1306_I2C(128, 64, i2c, 0x3C)
#oled1 = OLED.SSD1306_I2C(128, 64, i2c, 0x3D)

oled0.poweron()
#oled1.poweron()

oled0.text("screen 0", 0, 0)
#oled1.text("screen 1", 0, 0)

oled0.show()
#oled1.show()
