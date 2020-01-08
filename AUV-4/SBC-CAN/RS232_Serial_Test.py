import serial
import time
print("hello")

#change port name if required
ser = serial.Serial('/dev/cu.usbserial-14430',115200,timeout = 5)

tick = time.clock_gettime_ns(time.CLOCK_REALTIME)
count = 0
while(count < 1):	#send 1 packet only
	if(time.clock_gettime_ns(time.CLOCK_REALTIME) - tick > 1000*3000): #3ms
		tick = time.clock_gettime_ns(time.CLOCK_REALTIME)
		# Modify the array for different packets
		# START,START,ID,LEN,MSG(1-8)
		arr =[254,254,3,1,8] 
		msg = bytearray(arr)
		ser.write(msg)
		count +=1

