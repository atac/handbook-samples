#!/usr/bin/env python

from array import array
from cStringIO import StringIO
from thread import start_new_thread
from wsgiref.simple_server import make_server
import json

from bitstring import BitArray
import dpkt
import pcap

from chapter10 import C10


# The packet queue.
packets = []
#packets = [p for p in C10('in.c10')]
#print 'Loaded'
batch_size = 100

BUFFER_SIZE = 10000
BUFFER = ''


def webapp(environ, start_response):
    global packets
    headers = {'Content-Type': 'text/html'}
    path = environ['PATH_INFO']
    if path.endswith('.css') or path.endswith('.js'):
        with open('display/' + path[1:], 'r') as f:
            resp = f.read()
        headers['Content-Type'] = path.endswith('css') and 'text/css' or \
            'text/javascript'
    elif path == '/index.json':
        data, channels = [], {}
        if len([p for p in packets if p.data_type == 9]) >= batch_size:
            print 'sending batch'
            for packet in packets:
                if packet.channel_id not in channels:
                    channels[packet.channel_id] = {'type': packet.data_type,
                                                   'id': packet.channel_id,
                                                   'size': 0}
                channels[packet.channel_id]['size'] += 1

                if packet.data_type == 0x9:
                    a = array('H', ''.join([f.data for f in packet.body.frames]))
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
            packets = []
        resp = json.dumps({'pcm': data, 'channels': channels})
        headers['Content-Type'] = 'application/json'
    else:
        with open('display/index.html') as f:
            resp = f.read()
    start_response('200 OK', headers.items())
    return resp


def serve():
    """Data display webserver."""

    server = make_server('0.0.0.0', 8080, webapp)
    server.serve_forever()


def handle_packet(ts, packet):
    """Add packet to the stack."""

    global BUFFER, packets
    eth = dpkt.ethernet.Ethernet(str(packet))
    packet = eth.data.data.data
    BUFFER += packet
    if len(BUFFER) > BUFFER_SIZE:
        packets += [p for p in C10(StringIO(BUFFER))]
        BUFFER = ''
    with open('out.c10', 'ab') as out:
        out.write(packet)


def main():
    """Listen for data and serve display until interrupted."""

    start_new_thread(serve, ())
    while True:
        pc = pcap.pcap()
        pc.setfilter('udp and dst host 192.168.1.120 and dst port 2000')
        pc.loop(handle_packet)

if __name__ == '__main__':
    main()
