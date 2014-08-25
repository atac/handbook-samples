#!/usr/bin/env python

__doc__ = """usage: stat.py <file> [options]

Options:
    -c CHANNEL..., --channel CHANNEL...  Specify channels to include(csv).
    -e CHANNEL..., --exclude CHANNEL...  Specify channels to ignore (csv).
    -t TYPE, --type TYPE                 The types of data to show (csv, may \
be decimal or hex eg: 0x40)."""

from docopt import docopt

from chapter10 import C10
from chapter10.datatypes import get_label

from walk import walk_packets


if __name__ == '__main__':

    # Get commandline args.
    args = docopt(__doc__)

    channels, packets, size = ([], 0, 0)

    # Open the source file.
    src = C10(args['<file>'])

    # Iterate over selected packets (based on args).
    for packet in walk_packets(src, args):

        # Increment overall size and packet count.
        size += packet.packet_length
        packets += 1

        # Find the channel info based on ID and type.
        channel_index = None
        for i, channel in enumerate(channels):
            if channel['id'] == packet.channel_id and \
                    channel['type'] == packet.data_type:
                channel_index = i
                break

        # Create a listing if none exists.
        if channel_index is None:
            channel_index = len(channels)
            channels.append({'packets': 0,
                            'type': packet.data_type,
                            'id': packet.channel_id})

        # Increment the counter.
        channels[channel_index]['packets'] += 1

    # Print details for each channel.
    print('Channel ID     Data Type' + 'Packets'.rjust(46))
    print('-' * 80)
    for channel in channels:
        print (''.join((('Channel %s' % channel['id']).ljust(15),
                       ('%s - %s' % (hex(channel['type']),
                                     get_label(channel['type']))).ljust(35),
                       ('%s packets' % channel['packets']).rjust(20))))

    # Find a more readable size unit than bytes.
    units = ['gb', 'mb', 'kb']
    unit = 'b'
    while size > 1024 and units:
        size /= 1024.0
        unit = units.pop()

    # Print file summary.
    print('-' * 80)
    print('Summary for %s:' % args['<file>'])
    print('    Size: %s %s' % (round(size, 2), unit))
    print('    Packets: %s' % packets)
    print('    Channels: %s' % len(channels))
