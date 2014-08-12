"""
IRIG 106 Data DLL - This module provides an interface to the IRIG 106 data DLL.

The IRIG 106 DLL irig106.dll must be present somewhere in the system where
Windows can find it.

Message data structures are based on the ctypes module.  The main implication
of this is that to use data first the data buffer needs to be cast to the
appropriate data structure.  Then the fields are accessed using the
'.contents' attribute.
"""

import os.path
import sys
import ctypes
import Status

# ---------------------------------------------------------------------------
# IRIG 106 data structures
# ---------------------------------------------------------------------------


class Header(ctypes.Structure):
    """Data structure for IRIG 106 packet primary and secondary header."""

    _pack_ = 1
    _fields_ = [("Sync", ctypes.c_uint16),
                ("ChID", ctypes.c_uint16),
                ("PacketLen", ctypes.c_uint32),
                ("DataLen", ctypes.c_uint32),
                ("HdrVer", ctypes.c_uint8),
                ("SeqNum", ctypes.c_uint8),
                ("PacketFlags", ctypes.c_uint8),
                ("DataType", ctypes.c_uint8),
                ("RefTime", ctypes.c_uint8 * 6),
                ("Checksum", ctypes.c_uint16),
                ("Time", ctypes.c_uint32 * 2),
                ("Reserved", ctypes.c_uint16),
                ("SecChecksum", ctypes.c_uint16)]

# ---------------------------------------------------------------------------
# IRIG 106 constants
# ---------------------------------------------------------------------------


class FileMode():
    """Data file open mode."""

    CLOSED = 0
    READ = 1
    OVERWRITE = 2  # Create a new file or overwrite an exising file
    APPEND = 3
    READ_IN_ORDER = 4  # Open an existing file for reading in time order
    READ_NET_STREAM = 5  # Open network data stream


class DataType(object):
    """Packet Message Types."""

    # List of tuples of format (var, label, value).
    types = [('USER_DEFINED', 'User Defined', 0),
             ('COMPUTER_0', None, 0),
             ('COMPUTER_1', None, 1),
             ('TMATS', 'TMATS', 1),
             ('COMPUTER_2', None, 2),
             ('RECORDING_EVENT', 'Event', 2),
             ('COMPUTER_3', None, 3),
             ('RECORDING_INDEX', 'Index', 3),
             ('COMPUTER_4', 'Computer Generated 4', 4),
             ('COMPUTER_5', 'Computer Generated 5', 5),
             ('COMPUTER_6', 'Computer Generated 6', 6),
             ('COMPUTER_7', 'Computer Generated 7', 7),
             ('PCM_FMT_0', 'PCM Format 0', 8),
             ('PCM_FMT_1', 'PCM Format 1', 9),
             ('IRIG_TIME', 'Time', 17),
             ('MIL1553_FMT_1', '1553', 25),
             ('MIL1553_FMT_2', '16PP194', 26),
             ('ANALOG', 'Analog', 33),
             ('DISCRETE', 'Discrete', 41),
             ('MESSAGE', 'Message', 48),
             ('ARINC_429_FMT_0', 'ARINC 429', 56),
             ('VIDEO_FMT_0', 'Video Format 0', 64),
             ('VIDEO_FMT_1', 'Video Format 1', 65),
             ('VIDEO_FMT_2', 'Video Format 2', 66),
             ('IMAGE_FMT_0', 'Image Format 0', 72),
             ('IMAGE_FMT_1', 'Image Format 1', 73),
             ('UART_FMT_0', 'UART', 80),
             ('IEEE1394_FMT_0', 'IEEE 1394 Format 0', 88),
             ('IEEE1394_FMT_1', 'IEEE 1394 Format 1', 89),
             ('PARALLEL_FMT_0', 'Parallel', 96),
             ('ETHERNET_FMT_0', 'Ethernet', 104),
             ('TSPI_CTS_FMT_0', 'GPS NMEA-RTCM', 112),
             ('TSPI_CTS_FMT_1', 'EAG ACMI', 113),
             ('TSPI_CTS_FMT_2', 'ACTTS', 114),
             ('CAN_BUS', 'CAN Bus', 120)]

    @classmethod
    def name(cls, num):
        """Find the label for a format number."""

        for attr, label, val in cls.types:
            if num == val and label is not None:
                return label


# Populate DataType attirbutes from types list.
for attr, label, val in DataType.types:
    setattr(DataType, attr, val)


# ---------------------------------------------------------------------------
# Direct calls into the IRIG 106 dll
# ---------------------------------------------------------------------------

def I106_Ch10Open(file_name, file_mode):
    """Open IRIG 106 Ch 10 data file.
    file_mode should be Py106.FileMode attribute.
    """

    handle = ctypes.c_int32(0)
    status = IrigDataDll.enI106Ch10Open(ctypes.byref(handle),
                                        file_name, file_mode)
    return status, handle


def I106_Ch10Close(handle):
    ''' Close IRIG 106 Ch 10 data file '''
    # handle - IRIG file handle
    ret_status = IrigDataDll.enI106Ch10Close(handle)
    return ret_status


def I106_Ch10ReadNextHeader(handle, pkt_header):
    ''' Read next packet header '''
    # handle - IRIG file handle
    # pkt_header - Py106 Header() class, mutable
    ret_status = IrigDataDll.enI106Ch10ReadNextHeader(handle, ctypes.byref(pkt_header))
    return ret_status


def I106_Ch10ReadPrevHeader(handle, pkt_header):
    ''' Read previous packet header '''
    # handle - IRIG file handle
    # pkt_header - Py106 class Header(), mutable
    ret_status = IrigDataDll.enI106Ch10ReadPrevHeader(handle, ctypes.byref(pkt_header))
    return ret_status


def I106_Ch10ReadData(handle, buff_size, data_buff):
    # handle - IRIG file handle
    # buff_size - Size of data_buff
    # data_buff - Ctypes string buffer, mutable
    ret_status = IrigDataDll.enI106Ch10ReadData(handle, buff_size, ctypes.byref(data_buff))
    return ret_status


def I106_Ch10SetPos(handle, offset):
    # handle - IRIG file handle
    # offset - file offset
    ret_status = IrigDataDll.enI106Ch10SetPos(handle, offset)
    return ret_status


def I106_Ch10GetPos(handle):
    # handle - IRIG file handle
    offset = ctypes.c_uint64(0)
    ret_status = IrigDataDll.enI106Ch10GetPos(handle, ctypes.byref(offset))
    return (ret_status, offset.value)


# ---------------------------------------------------------------------------
# IRIG IO class
# ---------------------------------------------------------------------------

class IO(object):
    '''
    IRIG 106 packet data input / output
    '''

    # Constructor
    # -----------

    def __init__(self):
        self._Handle = ctypes.c_uint32(-1)
        self.Header  = Header()
        self.Buffer  = ctypes.create_string_buffer(0)


    # Open and close
    # --------------

    def open(self, Filename, FileMode):
        ''' Open an IRIG file for reading or writing '''
        RetStatus, self._Handle = I106_Ch10Open(Filename, FileMode)
        return RetStatus


    def close(self):
        ''' Close an open IRIG file '''
        RetStatus = I106_Ch10Close(self._Handle)
        return RetStatus


    # Read / Write
    # ------------

    def read_next_header(self):
        RetStatus = I106_Ch10ReadNextHeader(self._Handle, self.Header)
        return RetStatus


    def read_prev_header(self):
        RetStatus = I106_Ch10ReadPrevHeader(self._Handle, self.Header)
        return RetStatus


    def read_data(self):
        if self.Header.PacketLen > self.Buffer._length_:
            self.Buffer   = ctypes.create_string_buffer(self.Header.PacketLen)
        RetStatus = I106_Ch10ReadData(self._Handle, self.Buffer._length_, self.Buffer)
        return RetStatus


    def packet_headers(self):
        ''' Iterator of individual packet headers '''
        RetStatus = self.read_next_header()
        while RetStatus == Status.OK:
            yield self.Header
            RetStatus = self.read_next_header()


    # Other utility functions
    # -----------------------
    def set_pos(self, offset):
        ret_status = I106_Ch10SetPos(self._Handle, offset)
        return ret_status


    def get_pos(self):
        (ret_status, offset) = I106_Ch10GetPos(self._Handle)
        return (ret_status, offset)


# ---------------------------------------------------------------------------
# Module initialization
# ---------------------------------------------------------------------------

# Make the IRIG 106 DLL ctypes object
FilePath, FileName = os.path.split(os.path.abspath(__file__))
DllFileName = os.path.join(FilePath, "irig106.dll")
IrigDataDll = ctypes.cdll.LoadLibrary(DllFileName)

# This test code just opens an IRIG file and does a histogram of the
# data types

if __name__=='__main__':
    print "IRIG 1106 PacketIO"
    PktIO = IO()

    # Initialize counts variables
    Counts = {}

    if len(sys.argv) > 1 :
        RetStatus = PktIO.open(sys.argv[1], FileMode.READ)
        if RetStatus != Status.OK :
            print "Error opening data file %s" % (sys.argv[1])
            sys.exit(1)
    else :
        print "Usage : Packet.py <filename>"
        sys.exit(1)

#    The old traditional (aka FORTRAN) way of doing it
#    while True:
#        RetStatus = PktIO.read_next_header()
#        if RetStatus != Status.OK:
#            break
#        if Counts.has_key(PktIO.Header.DataType):
#           Counts[PktIO.Header.DataType] += 1
#        else:
#            Counts[PktIO.Header.DataType]  = 1

    # Using Python iteration
    for PktHdr in PktIO.packet_headers():
        if Counts.has_key(PktHdr.DataType):
            Counts[PktHdr.DataType] += 1
        else:
            Counts[PktHdr.DataType]  = 1

    PktIO.close()

    for DataTypeNum in Counts:
        print "Data Type %-16s Counts = %d" % ( DataType.name(DataTypeNum),  Counts[DataTypeNum])




