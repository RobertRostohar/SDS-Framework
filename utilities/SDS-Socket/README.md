# SDS-Socket
Python based socket server for the host PC.

It captures SDS recorder data sent from the target via socket
and writes recordings to files on the host.

Works together with the matching [sdsio_socket.c](../../sds/source/sdsio_socket.c) implementation on the target.

Sensor data is recorded to files `<sensor_name>.<index>.sds`:
 - `<sensor_name>` is the sensor name specified from the target
 - `<index>` is the zero-based index which is incremented for each subsequent recording

## Prerequisites

Target and PC must be connected to the same local network.

### Tools:
- [python 3.9 or later](https://www.python.org/downloads/windows/)

## Usage

The server is started by running the following command:
```
python sds-socket.py
```

>Note: The server is stopped when terminating the terminal which started the server!
