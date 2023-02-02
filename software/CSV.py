#import machine
#import SD

#an abstraction of a CSV file, used for logging sensor data

class CSV():
    
    def __init__(self, x = 10, y = 10, fname = 'log', directory = 'logs'):
        self.x = x
        self.y = y
        array = []
        for i in range(self.y):
            b = []
            for j in range(self.x):
                b = b + ['']
            array = array + [b]
        self.array = array
        self.fname = fname
        self.directory = directory
        self.sd = SD.card(self.fname, self.directory)
        
        
    def comp(self):
        return(0)
        #return a CSV-formatted string for writing to the card
        
    def write(self, fname = ''):
        return(0)
        #compile then write to SD card
        
    def __str__(self):
        ret = ''
        for i in range(self.y):
            ret = ret + str(self.array[i]) + '\n'
        return(ret)