#!/usr/bin/python3
'''
Subclasses BluetoothSocket to serve messages "LOW" and "HIGH" based on values received from
client

Copyright Simon D. Levy 2018

MIT License
'''

import os
import time
from bluetooth_server import BluetoothServer

class LowHighServer(BluetoothServer):

    def __init__(self):

        BluetoothServer.__init__(self)

    def handleMessage(self, message):
        print ("Recibi" + message)
        if (message == "unlock"):
            print("Desbloqueando")
            os.system("echo \"high 4\" > /dev/city_bike")
            time.sleep(10)
            os.system("echo \"low 4\" > /dev/city_bike")
        
        self.send('Hellow from server')

if __name__ == '__main__':

    server = LowHighServer()

    server.start()
