#!/usr/bin/env python

__doc__ = """usage: i106_stat.py <file> [options]

Options:
    -c CHANNEL..., --channel CHANNEL...  Specify channels to include(csv).
    -e CHANNEL..., --exclude CHANNEL...  Specify channels to ignore (csv).
    -t TYPE, --type TYPE                 The types of data to show (csv, may \
be decimal or hex eg: 0x40)."""

from contextlib import closing

from docopt import docopt

from Py106.Packet import IO, FileMode, Status, DataType
from walk import walk_packets


if __name__ == '__main__':
    args = docopt(__doc__)

    channels, packets, size = ([], 0, 0)

    with closing(IO()) as PktIO:
        RetStatus = PktIO.open(args['<file>'], FileMode.READ)
        if RetStatus != Status.OK:
            print "Error opening data file %s" % args['<file>']
            raise SystemExit

        for packet in walk_packets(PktIO.packet_headers(), args):
            size += packet.PacketLen
            packets += 1
            channel_index = None
            for i, channel in enumerate(channels):
                if channel['id'] == packet.ChID and \
                        channel['type'] == packet.DataType:
                    channel_index = i
                    break
            if channel_index is None:
                channel_index = len(channels)
                channels.append({'packets': 0,
                                 'type': packet.DataType,
                                 'id': packet.ChID})
            channels[channel_index]['packets'] += 1

    print('Channel ID      Data Type' + 'Packets'.rjust(47))
    print('-' * 80)
    for channel in channels:
        print (''.join((('Channel %s' % channel['id']).ljust(15),
                       ('%s - %s' % (hex(channel['type']),
                                     DataType.name(channel['type']))).ljust(35),
                       ('%s packets' % channel['packets']).rjust(20))))

    units = ['gb', 'mb', 'kb']
    unit = 'b'
    while size > 1024 and units:
        size /= 1024.0
        unit = units.pop()

    print('-' * 80)
    print('Summary for %s:' % args['<file>'])
    print('    Size: %s %s' % (round(size, 2), unit))
    print('    Packets: %s' % packets)
    print('    Channels: %s' % len(channels))
