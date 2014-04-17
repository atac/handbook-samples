#!/usr/bin/env python

__doc__ = """usage: dump.py <file> [options]

Options:
    -o OUT, --output OUT                 The directory to place files \
[default: .].
    -c CHANNEL..., --channel CHANNEL...  Specify channels to include(csv).
    -e CHANNEL..., --exclude CHANNEL...  Specify channels to ignore (csv).
    -t TYPE, --type TYPE                 The types of data to export (csv, may\
 be decimal or hex eg: 0x40)
    -f, --force                          Overwrite existing files."""

from array import array
from contextlib import closing
import atexit
import os

from docopt import docopt

from Py106.Packet import IO, FileMode, Status


if __name__ == '__main__':
    args = docopt(__doc__)

    # Ensure OUT exists.
    if not os.path.exists(args['--output']):
        os.makedirs(args['--output'])

    with closing(IO()) as PktIO:
        RetStatus = PktIO.open(args['<file>'], FileMode.READ)
        if RetStatus != Status.OK:
            print "Error opening data file %s" % args['<file>']
            raise SystemExit

        out = {}
        while RetStatus == Status.OK:
            RetStatus = PktIO.read_next_header()
            if PktIO.read_data() != Status.OK:
                continue
            header = buffer(PktIO.Header)[:]
            if not bool(PktIO.Header.PacketFlags & (1 << 7)):
                header = header[:-12]

            # Get filename
            filename = os.path.join(args['--output'], str(PktIO.Header.ChID))

            if 0x3F < PktIO.Header.DataType < 0x43:
                filename += '.mpg'

            # Ensure an file is open (and will close) for a given channel.
            if filename not in out:

                # Don't overwrite without explicit direction.
                if os.path.exists(filename) and not args['--force']:
                    print('%s already exists. Use -f to overwrite.' % filename)
                    break

                out[filename] = open(filename, 'wb')
                atexit.register(out[filename].close)

            out[filename].write(header)

            data = PktIO.Buffer.raw[:PktIO.Header.PacketLen - len(header)]
            if 0x3F < PktIO.Header.DataType < 0x43:
                for i in range(len(data) / 188):
                    body = array('H', data[i * 188:(i + 1) * 188])
                    body.byteswap()
                    out[filename].write(body.tostring())

            else:
                # Write out raw packet body.
                out[filename].write(data)
