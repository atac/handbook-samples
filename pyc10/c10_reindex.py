#!/usr/bin/env python

__doc__ = """usage: c10_reindex.py <src> <dst> [options]

Options:
    -s, --strip  Strip existing index packets and exit.
    -f, --force  Overwrite existing files."""

import os
import struct

from chapter10 import C10
from docopt import docopt

from walk import walk_packets


def gen_node(packets, seq=0):
    """Generate an index node packet."""

    print 'Index node for %s packets' % len(packets)

    packet = ''

    # Header
    values = [0xeb25,
              0,
              24 + 4 + (20 * len(packets)),
              4 + (20 * len(packets)),
              0x06,
              seq,
              0,
              0x03,
              packets[-1].rtc_low,
              packets[-1].rtc_high]
    values.append(sum(values) & 0xffff)
    packet += struct.pack('HHIIBBBBIHH', *values)

    # CSDW
    csdw = 0x0000
    csdw &= 1 << 31
    csdw &= 1 << 30
    csdw += len(packets)
    packet += struct.pack('I', csdw)

    # File Length (at start of node)
    pos = packets[-1].pos + packets[-1].packet_length
    packet += struct.pack('Q', pos)

    # Packets
    for p in packets:
        ipts = struct.pack('HH', p.rtc_low & 0xffff, p.rtc_high & 0xffff)
        index = struct.pack('xHHQ', p.channel_id, p.data_type, p.pos)
        packet += ipts + index

    return pos, packet


def gen_root(nodes):
    #@todo: generate and write root index packet
    print 'Root index for: %s nodes' % len(nodes)
    return 1


if __name__ == '__main__':
    args = docopt(__doc__)

    # Don't overwrite unless explicitly required.
    if os.path.exists(args['<dst>']) and not args['--force']:
        print('dst file already exists. Use -f to overwrite.')
        raise SystemExit

    with open(args['<dst>'], 'wb') as out:

        # Packets for indexing.
        packets, nodes = [], []
        node_seq = 0
        last_root = None
        for packet in walk_packets(C10(args['<src>']), args):
            if packet.data_type == 0x03:
                continue

            raw = bytes(packet)
            if len(raw) == packet.packet_length:
                out.write(raw)

            # Just stripping existing indices so move along.
            if args['--strip']:
                continue

            packets.append(packet)

            # Projected index node packet size.
            size = 24 + 4 + (20 * len(packets))
            if packet.data_type in (0x02, 0x11) or size > 524000:
                offset, raw = gen_node(packets)
                nodes.append(offset)
                out.write(raw)
                packets = []

                # Projected root index packet size.
                size = 24 + 4 + (16 * len(nodes)) + 8
                if size > 524000:
                    gen_root(nodes)
                    nodes = []

        # Final indices.
        if packets:
            nodes.append(gen_node(packets))
        if nodes:
            gen_root(nodes)

    if args['--strip']:
        print 'Stripped existing indices.'
