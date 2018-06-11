# Geostar Libary
## Supported systems
tested on the moment on 
    -64bit linux
    -avr m0

it should work on all other avr systems, but isn't tested so far.

## Funcionality
The libary handles a data stream by adding new data to a ringbuffer. While
doing so the libary privdes functions to check for the begnning of a
new dataset, hand out the positions and provide libarys to save store
the transmitted GPS data. As far supported are the datasets 0x20, 0x21
and 0x22.

## Knowm bugs
- Checksum check is not properly implemented yet
- gsParseGetTime gives wrong time values
- rounding issue with convFloat

