import can
import os
import time
import random
import signal
from itertools import count

DEVICES = 10

def createDevices():
    res = []
    for n in range(DEVICES):
        os.system(f"sudo ip link add dev testCan{n} type vcan")
        os.system(f"sudo ip link set dev testCan{n} up")
        res.append(can.interface.Bus(f"testCan{n}", bustype="virtual"))
    return res

def deleteDevices():
    for n in range(DEVICES):
        os.system(f"sudo ip link delete dev testCan{n}")

def interruptHandler(sig, frame):
    print("Deleting devices ...")
    deleteDevices()
    exit(0)

signal.signal(signal.SIGINT, interruptHandler)

if __name__ == "__main__":
    devices = createDevices()

    (device, n) = random.choice(list(zip(devices, count(0))))

    print(f"Sending on testCan{n}")
    print(f"Press C-c to interrupt")

    while(True):
        msg = can.Message(arbitration_id=0x5E9, data=[0x00]*8)
        device.send(msg)
        time.sleep(0.1)
