'''
Temperature Sensor AD7414

Current Address is 77

Address only changes upon power cycling

To use I2C Bus 2, in super user
>>sudo echo BB-I2C1 > /sys/devices/bone_capemgr.*/slots
This enables I2C Bus 2 to work. The mapping of the i2c buses are weird.
Currently Bus number is set to 1
'''

from Adafruit_I2C import Adafruit_I2C

class AD7414(object, Adafruit_I2C):

        #Tempreature
        #Bus used on BBB is bus 2
        def __init__(self, Add_Sel=72, busnum = 1, debug=False, hires=True):
                self.Add_Sel = Add_Sel
                self.busnum = busnum
                #creates an instance of I2C
                self.Temperature = Adafruit_I2C(self.Add_Sel, self.busnum, debug)

        def readTemp(self):
                #read from register 0, device will reply with 2 bytes
                self.Temperature.write8(self.Add_Sel,0)
                result = self.Temperature.readList(0,2)
                #Combined into a float
                temp = result[0]<<8
                temp = temp + result[1]
                return temp

        def calculateTemp(self):
                temp = self.readTemp()
                #Calculation is based of the data sheet
                temp = temp >> 6
                check = temp & 0b1000000000
                if  check > 0 :#Negative tempreature
                        #Highly unlikely this will ever happen.
                        print("Tempreature is Negative!!!")
                        final = (temp - 512)/4.0
                else:#Positive tempreature
                        final = temp/4.0