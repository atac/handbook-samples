#!/usr/bin/env python

from __future__ import print_function
import sys

try:
    from i106 import C10
except ImportError:
    from chapter10 import C10

from common import fmt_number, fmt_size, FileProgress


TYPES = (
    'Computer Generated',
    'PCM',
    'Time',
    'Mil-STD-1553',
    'Analog',
    'Discrete',
    'Message',
    'ARINC 429',
    'Video',
    'Image',
    'UART',
    'IEEE-1394',
    'Parallel',
    'Ethernet',
    'TSPI/CTS Data',
    'Controller Area Network Bus',
)


if __name__ == '__main__':

    # Get commandline args.
    if len(sys.argv) < 2:
        print('usage: stat.py <file>')
        raise SystemExit

    channels = {}

    with FileProgress(sys.argv[-1]) as progress:

        # Iterate over selected packets based on args.
        for packet in C10(sys.argv[-1]):
            key = (packet.channel_id, packet.data_type)
            if key not in channels:
                channels[key] = {'packets': 0,
                                 'size': 0,
                                 'type': packet.data_type,
                                 'id': packet.channel_id}

            channels[key]['packets'] += 1
            channels[key]['size'] += packet.packet_length

            progress.update(packet.packet_length)

    # Print details for each channel.
    print('{} {:>13} {:>38} {:>16}'.format(
        'Channel ID', 'Data Type', 'Packets', 'Size'))
    print('-' * 80)
    packets, size = 0, 0
    for key, channel in sorted(channels.items()):
        datatype = int(channel['type'] / 8.0)
        type_label = TYPES[datatype]
        subtype = channel['type'] - (datatype * 8)
        type_label = '{} - {} (format {})'.format(hex(channel['type']),
                                                  TYPES[datatype],
                                                  subtype)

        print('Channel {:<6} {:<40} {:>7} {:>16}'.format(
            channel['id'],
            type_label,
            fmt_number(channel['packets']),
            fmt_size(channel['size'])))

        packets += channel['packets']
        size += channel['size']

    # Print file summary.
    print('-' * 80)
    print('''Summary for {}:
    Channels: {:>10}
    Packets: {:>11}
    Size: {:>14}'''.format(
        sys.argv[-1], len(channels), fmt_number(packets), fmt_size(size)))
