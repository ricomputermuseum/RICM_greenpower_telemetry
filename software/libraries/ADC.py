import machine
from utime import sleep_ms

#Library for using a PCA7830 I2C ADC chip with a raspberry pi pico
#address range: 0x48-0x4D

#channel definitions, because the channel selection is coded to simplify the differentials
channels = [0,4,1,5,2,6,3,7]


class ADS7830():
    def __init__(self, i2c, address = 0x48):
        self.i2c = i2c
        self.address = address
        self.temp = bytearray(1)
        self.temp_read = bytearray(1)
        self.state = [0] * 16
        
    def read_channel_se(self, channel): # reads the value of a single channel relative to system ground
        cmd = 128
        cmd = cmd + (channels[channel]<<4)
        cmd = cmd + 4
        self.temp[0] = cmd
        self.i2c.writeto(self.address, self.temp)
        self.i2c.readfrom_into(self.address, self.temp_read)
        return(self.temp_read[0])
    
    def print_channels_se(self):
        for i in range(8):
            print(self.read_channel_se(i))
            
    def print_channels_diff(self):
        for i in range(8):
            print(self.read_channel_diff(i))
    
    def read_channel_diff(self, pair): #reads the differential value of two channels. The ADS7830 only allows differential readings on adjacent channels
        cmd = 0
        cmd = cmd + (channels[pair]<<4)
        cmd = cmd + 4
        self.temp[0] = cmd
        self.i2c.writeto(self.address, self.temp)
        self.i2c.readfrom_into(self.address, self.temp_read)
        return(self.temp_read[0])
        
    def read_channels_diff(self, ch0, ch1): #calculates a differential between an arbitrary pair of channels
        read_channel_se(ch0)
        x = self.temp_read[0]
        read_channel_se(ch1)
        y = self.temp_read[0]
        self.temp_read[0] = y-x
        return(self.temp_read[0])

#debug code
sda=machine.Pin(16)
scl=machine.Pin(17)
i2c0=machine.I2C(0, sda=sda, scl=scl, freq=400000)
vMonitor = ADS7830(i2c0)

def debug(n=255):
    for i in range(n):
        vMonitor.print_channels_se()
        print('')
        sleep_ms(1000)
    
