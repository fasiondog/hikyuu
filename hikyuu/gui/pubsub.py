"""
This pattern is used to allow a single broadcaster to publish messages to many subscribers,
which may choose to limit which messages they receive.
"""
import datetime
import time
import pynng
from hikyuu.gui.collect import *

address = "ipc:///tmp/pubsub.ipc"


def get_current_date():
    return str(datetime.datetime.now())


pub_sock = pynng.Pub0()
pub_sock.listen(address)
while True:
    #date = get_current_date()
    records = get_spot_parallel(['sh000001', 'sz000001'])
    print(len(records))
    buf = create_fb_spot(records)
    pub_sock.send(bytes(buf))
    #pub_sock.send(date.encode())
    time.sleep(3)
