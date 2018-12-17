# ecmR-library-for-Brultech-ECM-1240
Receives serial data from a Brultech ECM-1240 energy monitor, and provides routines for processing and reading the data.  Also makes the data available using Shared Memory so other applications can accesses the data.

This library provides the interface to the serial port, parses the ECM-1240 packets and stores the data in a memory array.  It provides routines for accessing data for individual channels as well as the data for AC voltage and (optionally) DC voltage measured by Aux channel 5.

The memory array can also be shared with other application using Posix Shared Memory.  Those applications can also link to this library to access the data for individual channels, etc. in the shared memory.

Note:  edd.c is included as an example of how to use this library.  It can store data from ecmR in both an RRD database and an ETSD database https://github.com/pvanderwal/ETSD-Time-Series-Database

<pre>
The memory array uses the following format:
Byte    Size    Value
Addr    bytes
0       2       DC Voltage on Aux 5
2       2       AC Voltage
4       5       Ch1 Absolute data (watts)
9       5       Ch2 Absolute data (watts)
14      5       Ch1 Polarity data (watts)
19      5       Ch2 Polarity data (watts)
24      4       Ser# etc. 
28      2       CH1 Current
30      2       CH2 Current
32      3       Seconds counter
35      1       [SHM Size]  // optional, used with second SHM object
36      4       Aux1  Watts
40      4       Aux2  Watts
44      4       Aux3  Watts
48      4       Aux4  Watts
52      4       Aux5  Watts
56      1       Interval 
57      1       Data inValid :        0=valid, 1=rxing new data, 2=initializing, 3=checksum, 4=timed out, 5=unspecified error
58      1       Calculated Checksum, or Packet position if timed out waiting for data
59      1       last byte received : normaly recieved Checksum unless packet timed out
60      4       Epoch time of current data
</pre>
See https://www.brultech.com/software/files/downloadSoft/ECM1240_Packet_format_ver9.pdf
for detailed information on the ECM1240 packet that this memory array is derived from.

