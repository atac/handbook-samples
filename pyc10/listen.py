#!/usr/bin/env python

from bitstring import BitArray
from cStringIO import StringIO
from thread import start_new_thread
from wsgiref.simple_server import make_server
import json
import time
import traceback

from scapy.all import *
from scapy.layers.inet import UDP
from chapter10 import C10
from chapter10.datatypes import format

from array import array

# The max number of packets kept in the queue.
packet_limit = 3000

# The packet queue.
packets = []
#packets = [p for p in C10('data/test.c10')]

batch_size = 500

BUFFER_SIZE = 1000000
BUFFER = ''


def webapp(environ, start_response):
    headers = {'Content-Type': 'text/html'}
    path = environ['PATH_INFO']
    if path.endswith('.css') or path.endswith('.js'):
        with open('display/' + path[1:], 'r') as f:
            resp = f.read()
        headers['Content-Type'] = path.endswith('css') and 'text/css' or \
            'text/javascript'
    elif path == '/index.json':
        data, channels = [], {}
        global BUFFER
        #if len(packets) > batch_size:
        if len(BUFFER) > BUFFER_SIZE:
            packets = [p for p in C10(StringIO(BUFFER))]
            BUFFER = ''
            #pcm = [packets.pop(0) for i in range(batch_size)]
            pcm = [p for p in packets if p.data_type == 0x9]
            a = array('H', ''.join([p.raw()[28:] for p in pcm
                                    if p.channel_id == 44]))
            a.byteswap()
            arr = BitArray(bytes=a.tostring())
            for i in arr.findall('0xfe6b2840'):
                seconds = arr[i + 96:i + 122]
                micro = arr[i + (16 * 8):i + (16 * 9)]
                ramp = arr[i + (16 * 15): i + (16 * 16)]
                seconds.byteswap()
                micro.byteswap()
                data.append({
                    'seconds': '{}.{}'.format(seconds.int, micro.int),
                    'ramp': ramp.uint})
            channels = {}
            for packet in pcm:
                if packet.channel_id not in channels:
                    channels[packet.channel_id] = {'type': packet.data_type,
                                                   'id': packet.channel_id,
                                                   'size': 1}
                else:
                    channels[packet.channel_id]['size'] += 1
        resp = json.dumps({'pcm': data, 'channels': channels})
        headers['Content-Type'] = 'application/json'
    else:
        with open('display/index.html') as f:
            resp = f.read()
    start_response('200 OK', headers.items())
    return resp


def serve():
    """Data display webserver."""

    #server = make_server('0.0.0.0', 8000, webapp)
    server = make_server('0.0.0.0', 8080, webapp)
    server.serve_forever()


def parse(s):
    global packets
    packets += [p for p in C10(StringIO(s))]


def listen():
    """Pull data from incoming ethernet."""

    global packets
    s = sniff(filter='udp and dst 192.168.1.107 and port 2000',
              count=250)
    for eth in s:
        try:
            if isinstance(eth.payload.payload, UDP):
                global BUFFER
                BUFFER += str(eth.load)
                #parse(str(eth.load))
                with open('out.c10', 'ab') as f:
                    f.write(str(eth.load)[4:])

        except:
            traceback.print_exc()
    if len(packets) > packet_limit:
        packets = packets[-packet_limit:]
    time.sleep(0.25)


def main():
    """Listen for data and serve display until interrupted."""

    #start_new_thread(listen, ())
    start_new_thread(serve, ())
    while True:
        try:
            import signal

            def signal_handler(signum, frame):
                raise Exception("Timed out!")

            signal.signal(signal.SIGALRM, signal_handler)
            signal.alarm(10)   # Ten seconds
            try:
                listen()
            except KeyboardInterrupt:
                raise
            except:
                print 'Timed out'
            print 'Buffer at %s' % len(BUFFER)
            #print '{} packets in queue'.format(len(packets))
        except KeyboardInterrupt:
            raise SystemExit

if __name__ == '__main__':
    main()
