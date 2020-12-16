"""
This pattern is used to allow a single broadcaster to publish messages to many subscribers,
which may choose to limit which messages they receive.
"""
import datetime
import time
import pynng
from hikyuu.gui.collect import *

address = "ipc:///tmp/hikyuu_real_pub.ipc"


def get_current_date():
    return str(datetime.datetime.now())


def client(name, max_msg=3):
    with pynng.Sub0() as sock:
        sock.subscribe("")
        sock.dial(address)

        while max_msg:
            msg = sock.recv_msg()
            #print(f"CLIENT ({name}): RECEIVED  {msg.bytes.decode()}")
            print_spot_list(bytearray(msg.bytes))
            max_msg -= 1


try:
    client("222222222222", 5)
except KeyboardInterrupt:
    exit(1)