'''

'''
from AD7414 import AD7414
from time import time, sleep

ads = AD7414(Add_Sel = 77)

while True:
        print("Temperature:")
        print(ads.calculateTemp())
        sleep(5)