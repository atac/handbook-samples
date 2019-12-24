#!/usr/bin/env python

"""usage: copy <src> <dst> [options]

Options:
    -c CHANNEL..., --channel CHANNEL...  Specify channels to include (comma \
separated).
    -e CHANNEL..., --exclude CHANNEL...  Specify channels to ignore (comma \
separated).
    -t TYPE, --type TYPE                 The types of data to copy (comma \
separated, may be decimal or hex eg: 0x40)
    -f --force                           Overwrite existing files."""

import os

try:
    from i106 import C10
except ImportError:
    from chapter10 import C10
from docopt import docopt

from common import walk_packets, FileProgress


if __name__ == '__main__':

    # Get commandline args.
    args = docopt(__doc__)

    # Don't overwrite unless explicitly required.
    if os.path.exists(args['<dst>']) and not args['--force']:
        print('dst file already exists. Use -f to overwrite.')
        raise SystemExit

    # Open input and output files.
    with open(args['<dst>'], 'wb') as out, FileProgress(args['<src>']) \
            as progress:

        # Iterate over packets based on args.
        for packet in walk_packets(C10(args['<src>']), args):

            progress.update(packet.packet_length)

            # Copy packet to new file.
            raw = bytes(packet)
            if len(raw) == packet.packet_length:
                out.write(raw)
