# SDS File Format

The **SDS Framework** uses a binary data format to store the individual data streams.

The binary data consists of records (variable size). Each record contains:
 - timestamp: record timestamp in milliseconds (32-bit unsigned integer, little endian)
 - data_size: number of data bytes in the record (32-bit unsigned integer, little endian)
 - data: data stream in the record

The content of the data stream is described in a YAML metadata file that is created by the user.

## YML Format

The following section defines the YAML format of this metadata file. The file `sds.schema.json` is a schema description of the SDS Format Description.

`sds:`                               | Start of the SDS Format Description
:------------------------------------|---------------------------------------------------
&nbsp;&nbsp; `name:`                 | Name of the Synchronous Data Stream (SDS)
&nbsp;&nbsp; `description:`          | Additional descriptive text
&nbsp;&nbsp; `frequency:`            | Capture frequency of the SDS
&nbsp;&nbsp; `content:`              | List of values captured (see below)

`content:`                           | List of values captured (in the order of the data file)
:------------------------------------|---------------------------------------------------
&nbsp;&nbsp; `value:`                | Name of the value
&nbsp;&nbsp; `type:`                 | Data type of the value
&nbsp;&nbsp; `offset:`               | Offset of the value
&nbsp;&nbsp; `scale:`                | Scale factor of the value
&nbsp;&nbsp; `unit:`                 | Physical unit of the value

## Example

This example defines a data stream with the name "sensorX" that contains the values of a gyroscope, temperature sensor, and additional raw data (that are not further described).
The binary data that are coming form this sensors are stored in data files with the following file format: `<sensor-name>.<file-index>.sds`. In this example the files names could be:

```yml
   sensorX.0.sds   # capture 0
   sensorX.1.sds   # capture 1
```

The following `sensorX.sds.yml` provides the format description of the SDS `sensorX` binary data files and maybe used by data conversion utilities and data viewers.

```yml
sds:                   # describes a synchronous data stream
  name: sensorX        # user defined name
  description: Gyroscope stream with 1KHz, plus additional user data
  frequency: 1000
  content:
  - value: x           # Value name is 'x'
    type:  uint16_t    # stored using a 16-bit unsigned int
    scale: 0.2         # value is scaled by 0.2
    unit: dps          # base unit of the value
  - value: y
    type: uint16_t
    scale: 0.2
    unit: dps
  - value: z
    type: uint16_t
    unit: dps          # scale 1.0 is default
  - value: temp
    type: float
    unit: degree Celsius
  - value: raw
    type: uint16_t     # raw data, no scale or unit given
  - value: flag
    type: uint32_t:1   # a single bit stored in a 32-bit int
```
