import machine
import sdcard
import os

# an abstraction of an SD card

class card():
    def __init__(self, fname_default, directory):
        self.fname_default = fname_default #default filename
        self.sd_cs = machine.Pin(21) #sd chip select pin
        self.so = machine.Pin(20)    #spi controller tx
        self.si = machine.Pin(19)    #spi controller rx
        self.sck = machine.Pin(18)   #spi serial clock

        self.spi = machine.SPI(1, 500000, 0, 0, 8, machine.SPI.MSB, self.sck, self.so, self.si)
        self.sd = sdcard.SDCard(spi, sd_cs)
        
        vfs = os.VfsFat(sd)
        uos.mount(vfs, "/sd") #not entirely sure you can mount a filsystem inside a class constructor and access it after the call returns, may have to do this somewhere else to avoid repeatedly mounting and demounting
        x = os.listdir("/sd")
        if(('logs' in x) == False):
            os.mkdir("/sd/logs")
        self.current_file_number = self.get_current_file_number()
        
    def __str__(self):
        ret = ''
        for i in os.walk('sd/'):
            ret = ret + str(i) + '\n'
        
    def write_to_card(self, fname, data, directory = ''):
        file = open("/sd/"+fname, "a")
        file.write(str(data))
        file.close()
        
    def read_from_card(self, fname):
        file = open("/sd/"+fname, "r")
        ret = file.read()
        print(ret)
        file.close()
        return(ret)
    
    #utility for writing consecutive log files. names must be in the format fname_xxxx.yyy, where xxxx is the 4-digit number of the file, and yyy is the 3 digit file extension.
    def get_current_file_number(self, fname = self.fname_default, directory = 'logs'):
        length = len(fname)
        files = os.listdir('/sd/'+directory)
        number = 0
        for i in files:
            if(len(i) == length+9 and i[0:length] == fname): #checking for format fname_xxxx.yyy
                x = i[-8:-4]
                x = int(x)
                if(x > number):
                    number = x
        self.current_file_number = number
        return(self.current_file_number)
            
    def debug(self):
        print(os.listdir('/sd'))
    
   