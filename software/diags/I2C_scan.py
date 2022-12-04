from micropython import const
import machine
import utime

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