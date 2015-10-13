#!/usr/bin/env python

"""
  dump.py - Export channel data based on channel ID or data type.

 Copyright (c) 2015 Micah Ferrill

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may
     be used to endorse or promote products derived from this software
     without specific prior written permission.

 This software is provided by the copyright holders and contributors
 "as is" and any express or implied warranties, including, but not
 limited to, the implied warranties of merchantability and fitness for
 a particular purpose are disclaimed. In no event shall the copyright
 owner or contributors be liable for any direct, indirect, incidental,
 special, exemplary, or consequential damages (including, but not
 limited to, procurement of substitute goods or services; loss of use,
 data, or profits; or business interruption) however caused and on any
 theory of liability, whether in contract, strict liability, or tort
 (including negligence or otherwise) arising in any way out of the use
 of this software, even if advised of the possibility of such damage.
"""

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
from walk import walk_args


if __name__ == '__main__':
    args = docopt(__doc__)

    # Ensure OUT exists.
    if not os.path.exists(args['--output']):
        os.makedirs(args['--output'])

    channels, exclude, types = walk_args(args)

    # Open input file.
    with closing(IO()) as PktIO:
        RetStatus = PktIO.open(args['<file>'], FileMode.READ)
        if RetStatus != Status.OK:
            print "Error opening data file %s" % args['<file>']
            raise SystemExit

        out = {}

        # Iterate over packets based on args.
        while RetStatus == Status.OK:
            RetStatus = PktIO.read_next_header()

            if channels and str(PktIO.Header.ChID) not in channels:
                continue
            elif str(PktIO.Header.ChID) in exclude:
                continue
            elif types and PktIO.Header.DataType not in types:
                continue

            if PktIO.read_data() != Status.OK:
                continue

            # Get filename for this channel based on data type.
            filename = os.path.join(args['--output'], str(PktIO.Header.ChID))
            if 0x3F < PktIO.Header.DataType < 0x43:
                filename += '.mpg'

            # Ensure a file is open (and will close) for a given channel.
            if filename not in out:

                # Don't overwrite unless explicitly required.
                if os.path.exists(filename) and not args['--force']:
                    print('%s already exists. Use -f to overwrite.' % filename)
                    break

                out[filename] = open(filename, 'wb')
                atexit.register(out[filename].close)

            data = PktIO.Buffer.raw[4:PktIO.Header.PacketLen - 4]

            # Handle special case for video data.
            if bool(0x3F < PktIO.Header.DataType < 0x43):
                for i in range(len(data) / 188):
                    body = array('H', data[i * 188:(i + 1) * 188])
                    body.byteswap()
                    out[filename].write(body.tostring())

            else:
                # Write out raw packet body.
                out[filename].write(data)
