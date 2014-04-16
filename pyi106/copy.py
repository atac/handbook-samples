#!/usr/bin/env python

__doc__ = """usage: copy.py <src> <dst> [options]

Options:
    -c CHANNEL..., --channel CHANNEL...  Specify channels to include (csv).
    -e CHANNEL..., --exclude CHANNEL...  Specify channels to ignore (csv).
    -t TYPE, --type TYPE                 The types of data to copy (csv, may\
 be decimal or hex eg: 0x40)
    -f --force                           Overwrite existing files."""

from contextlib import closing
import os

from docopt import docopt

from Py106.Packet import IO, FileMode, Status


if __name__ == '__main__':
    args = docopt(__doc__)

    # Don't overwrite unless explicitly required.
    if os.path.exists(args['<dst>']) and not args['--force']:
        print('dst file already exists. Use -f to overwrite.')
        raise SystemExit

    with open(args['<dst>'], 'wb') as out, closing(IO()) as PktIO:
        RetStatus = PktIO.open(args['<src>'], FileMode.READ)
        if RetStatus != Status.OK:
            print "Error opening data file %s" % args['<src>']
            raise SystemExit

        while RetStatus == Status.OK:
            RetStatus = PktIO.read_next_header()
            if PktIO.read_data() != Status.OK:
                continue
            header = buffer(PktIO.Header)[:]
            if not bool(PktIO.Header.PacketFlags & (1 << 7)):
                header = header[:-12]
            out.write(header)
            out.write(PktIO.Buffer.raw[:PktIO.Header.PacketLen - len(header)])
