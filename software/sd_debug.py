import os
def get_current_file_number(directory = '', fname = 'hello'):
    length = len(fname)
    files = os.listdir('/'+directory)
    print(files)
    number = 0
    for i in files:
        if(len(i) == length+9 and i[0:length] == fname): #length + _xxxx.csv (9 chars)
            x = i[-8:-4]
            x = int(x)
            if(x > number):
                number = x
    current_file_number = number
    return(current_file_number)
