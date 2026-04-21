from asyncio import to_thread
import json
import time

from comms_ccf.channel import Channel

print(json.dumps([time.time(), "init"]))

async def add_event(chan: Channel, *args):
    await to_thread(lambda: print(json.dumps([time.time(), chan, *args])))
