#!/usr/bin/env python

__doc__ = """usage: c10_reindex.py <src> <dst> [options]

Options:
    -s, --strip  Strip existing index packets and exit.
    -f, --force  Overwrite existing files."""

from array import array
from contextlib import closing
import StringIO
import os
import struct

from docopt import docopt

from Py106.Packet import IO, FileMode, Status

from chapter10 import Packet


def gen_node(packets, seq=0):
    """Generate an index node packet."""

    # (rtc_low, rtc_high, channel_id, data_type, pos)

    print 'Index node for %s packets' % len(packets)

    packet = bytes()

    # Header
    values = [0xeb25,
              0,
              24 + 4 + (20 * len(packets)),
              4 + (20 * len(packets)),
              0x06,
              seq,
              0,
              0x03] + list(packets[-1][0])

    sums = struct.pack('HHIIBBBBBBBBBB', *values)
    sums = sum(array('H', sums)) & 0xffff
    values.append(sums)
    packet += struct.pack('HHIIBBBBBBBBBBH', *values)

    # CSDW
    csdw = 0x0000
    csdw &= 1 << 31
    csdw &= 1 << 30
    csdw += len(packets)
    packet += struct.pack('I', csdw)

    # File Length (at start of node)
    pos = packets[-1][-1] + packets[-1][3]
    packet += struct.pack('Q', pos)

    # Packets
    for p in packets:
        ipts = struct.pack('BBBBBB', *p[0])
        index = struct.pack('xHHQ', p[1], p[2], p[4])
        packet += ipts + index

    return pos, packet


def gen_root(nodes, last, seq, last_packet):
    """Generate a root index packet."""

    print 'Root index for: %s nodes' % len(nodes)

    packet = bytes()

    # Header
    values = [0xeb25,
              0,
              24 + 4 + 8 + 8 + (16 * len(packets)),
              4 + 8 + 8 + (16 * len(packets)),
              0x06,
              seq,
              0,
              0x03] + list(last_packet[0])

    sums = struct.pack('HHIIBBBBBBBBBB', *values)
    sums = sum(array('H', sums)) & 0xffff
    values.append(sums)
    packet += struct.pack('HHIIBBBBBBBBBBH', *values)

    # CSDW
    csdw = 0x0000
    csdw &= 1 << 30
    csdw += len(nodes)
    packet += struct.pack('I', csdw)

    # File Length (at start of node)
    pos = last_packet[-1] + last_packet[3]
    packet += struct.pack('Q', pos)

    # Node Packets
    for node in nodes:
        ipts = struct.pack('BBBBBB', *last_packet[0])
        offset = struct.pack('Q', pos - node)
        packet += ipts + offset

    if last is None:
        last = pos
    packet += struct.pack('Q', last)

    return pos, packet


def increment(seq):
    """Increment the sequence number or reset it."""

    seq += 1
    if seq > 0xFF:
        seq = 0
    return seq


if __name__ == '__main__':
    args = docopt(__doc__)

    # Don't overwrite unless explicitly required.
    if os.path.exists(args['<dst>']) and not args['--force']:
        print('dst file already exists. Use -f to overwrite.')
        raise SystemExit

    with open(args['<dst>'], 'wb') as out, closing(IO()) as io:

        status = io.open(args['<src>'], FileMode.READ)
        if status != Status.OK:
            print "Error opening data file %s" % args['<src>']
            raise SystemExit

        # Packets for indexing.
        packets, nodes = [], []
        node_seq = 0
        last_root = None
        last_packet = None

        while status == Status.OK:
            status = io.read_next_header()
            if io.read_data() != Status.OK:
                continue
            if io.Header.DataType == 0x03:
                continue

            header = buffer(io.Header)[:]
            out.write(header)
            raw = io.Buffer.raw[:io.Header.PacketLen - len(header)]
            out.write(raw)

            # Just stripping existing indices so move along.
            if args['--strip']:
                continue

            # (time, channel_id, data_type, length, pos)
            packet = (io.Header.RefTime, io.Header.Time[1],
                      io.Header.ChID, io.Header.DataType, io.Header.DataLen,
                      io.get_pos()[1] - io.Header.PacketLen)
            packets.append(packet)
            last_packet = packet

            # Projected index node packet size.
            size = 24 + 4 + (20 * len(packets))
            if io.Header.DataType in (0x02, 0x11) or size > 524000:
                offset, raw = gen_node(packets)
                nodes.append(offset)
                out.write(raw)
                packets = []

                # Projected root index packet size.
                size = 24 + 4 + (16 * len(nodes)) + 16
                if size > 524000:
                    last_root, raw = gen_root(nodes, last_root, node_seq,
                                              last_packet)
                    out.write(raw)
                    node_seq = increment(node_seq)
                    nodes = []

        # Final indices.
        if packets:
            offset, raw = gen_node(packets)
            nodes.append(offset)
            out.write(raw)
        if nodes:
            last_root, raw = gen_root(nodes, last_root, node_seq, last_packet)
            out.write(raw)

    if args['--strip']:
        print 'Stripped existing indices.'
