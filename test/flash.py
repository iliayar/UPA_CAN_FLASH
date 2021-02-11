#!/usr/bin/env python3
import can
import time


bus1 = can.interface.Bus(channel='vcan0', bustype='socketcan')

def work():
    global bus1
    
    msg1 = bus1.recv()
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x06, 0x50, 0x03, 0x00, 0x32, 0x01, 0xF4, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)
    
    msg2 = can.Message(arbitration_id=0x76e, data=[0x02, 0xC5, 0x02, 0x00, 0x32, 0x01, 0xF4, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x03, 0x68, 0x03, 0x03, 0x32, 0x01, 0xF4, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x06, 0x50, 0x02, 0x00, 0x32, 0x01, 0xF4, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x06, 0x67, 0x03, 0xAC, 0x2B, 0xAD, 0x07, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x02, 0x67, 0x04, 0xAC, 0x2B, 0xAD, 0x07, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x04, 0x74, 0x20, 0x04, 0x02, 0x00, 0x00, 0x00]);
    bus1.send(msg2)

    msg1 = bus1.recv();
    print(msg1)

    msg2 = can.Message(arbitration_id=0x76e, data=[0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]);
    bus1.send(msg2)

    i = 6
    while(i < 0x402):
        msg1 = bus1.recv()
        i += 7;
        
    print("Block received")


if __name__ == '__main__':
    while(True):
        work();
