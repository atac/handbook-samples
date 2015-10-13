#!/usr/bin/env python

"""
  copy.py - Copy all or part of a Chapter 10 file based on data types,
    channels, etc.

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
from walk import walk_args


if __name__ == '__main__':

    # Get commandline args.
    args = docopt(__doc__)

    # Don't overwrite unless explicitly required.
    if os.path.exists(args['<dst>']) and not args['--force']:
        print('dst file already exists. Use -f to overwrite.')
        raise SystemExit

    # Open input and output files.
    with open(args['<dst>'], 'wb') as out, closing(IO()) as PktIO:
        RetStatus = PktIO.open(args['<src>'], FileMode.READ)
        if RetStatus != Status.OK:
            print "Error opening data file %s" % args['<src>']
            raise SystemExit

        # Iterate over packets based on args.
        channels, exclude, types = walk_args(args)
        i = 0
        while RetStatus == Status.OK:
            RetStatus = PktIO.read_next_header()
            if i > 1:
                if PktIO.read_data() != Status.OK:
                    continue
                elif channels and str(PktIO.Header.ChID) not in channels:
                    continue
                elif str(PktIO.Header.ChID) in exclude:
                    continue
                elif types and PktIO.Header.DataType not in types:
                    continue

            # Copy packet to new file.
            header = buffer(PktIO.Header)[:]
            if not bool(PktIO.Header.PacketFlags & (1 << 7)):
                header = header[:-12]
            out.write(header)
            out.write(PktIO.Buffer.raw[:PktIO.Header.PacketLen - len(header)])

            i += 1
