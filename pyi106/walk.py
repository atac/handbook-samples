

def walk_args(args):
    """Parse args into channels, exclude, and types."""

    # Apply defaults.
    args['--type'] = args.get('--type') or ''
    args['--channel'] = args.get('--channel') or ''
    args['--exclude'] = args.get('--exclude') or ''

    # Parse types (if given) into ints.
    types = [t.strip() for t in args['--type'].split(',') if t.strip()]
    types = [int(t, 16) if t.startswith('0x') else int(t) for t in types]

    # Parse channel selection.
    channels = [c.strip() for c in args['--channel'].split(',') if c.strip()]
    exclude = [e.strip() for e in args['--exclude'].split(',') if e.strip()]

    return channels, exclude, types


def walk_packets(source, args={}):
    """Walk a chapter 10 file based on sys.argv (type, channel, etc.)."""

    channels, exclude, types = walk_args(args)

    for packet in source:
        if channels and str(packet.ChID) not in channels:
            continue
        elif str(packet.ChID) in exclude:
            continue
        elif types and packet.DataType not in types:
            continue

        yield packet
