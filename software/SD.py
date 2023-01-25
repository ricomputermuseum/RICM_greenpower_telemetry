import machine
import sdcard
import os

# wrapper for the SD card class that includes some additional utilities

class card():
    def __init__(self, fname_default='log'):
        self.fname_default = fname_default #default filename
        spi= machine.SPI(1, baudrate=40000000, sck=machine.Pin(10), mosi=machine.Pin(11), miso=machine.Pin(12))
        self.SD=sdcard.SDCard(spi, machine.Pin(13))
        
        self.vfs=os.VfsFat(self.SD)
        os.mount(self.SD, "/sd") #not entirely sure you can mount a filsystem inside a class constructor and access it after the call returns, may have to do this somewhere else to avoid repeatedly mounting and demounting
        x = os.listdir("/sd")
        if(('logs' in x) == False):
            os.mkdir("/sd/logs")
        self.current_file_number = self.get_current_file_number()
        
    def __str__(self):
        ret = ''
        x = os.listdir('/sd')
        y = os.listdir('/sd/logs')
        
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
    def get_current_file_number(self, fname = '', directory = 'logs'):
        if(fname == ''):
            fname = self.fname_default
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
    
   