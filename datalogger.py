import machine
import sdcard
import uos

# an abstraction of a datalogger. takes in data from the pico in a list and appends it to a file on the sd card

class datalogger():
    def __init__(self, fName):
        self.fName= fName #name of the file into which to log the data
        self.sd_cs = machine.Pin(21) #sd chip select pin
        self.so = machine.Pin(20)    #spi controller tx
        self.si = machine.Pin(19)    #spi controller rx
        self.sck = machine.Pin(18)   #spi serial clock

        self.spi = machine.SPI(1, 500000, 0, 0, 8, machine.SPI.MSB, self.sck, self.so, self.si)
        self.sd = sdcard.SDCard(spi, sd_cs)
        
        vfs = uos.VfsFat(sd)
        uos.mount(vfs, "/sd") #not entirely sure you can mount a filsystem inside a class constructor and access it after the call returns, may have to do this somewhere else to avoid repeatedly mounting and demounting
        
    def write_to_card(self, data, delimiter = "\r\n"):
        with open("/sd/"+self.fName, "w"):
            file.write(data.str()+delimiter)
            
    def write_row(self, data): #takes an array of values and writes them as a new row on a csv
        for i in data:
            self.write_to_card(i, " , ")
        self.write_to_card('')
        