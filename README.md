---
documentclass: physycomen
title:  "nmea_to_json"
author: "Di Cristina, Fabbri, Sinigardi"
---

<a href="http://www.physycom.unibo.it"> 
<div class="image">
<img src="https://cdn.rawgit.com/physycom/templates/697b327d/logo_unibo.png" width="90" height="90" alt="© Physics of Complex Systems Laboratory - Physics and Astronomy Department - University of Bologna"> 
</div>
</a>
<a href="https://travis-ci.org/physycom/nmea_to_json"> 
<div class="image">
<img src="https://travis-ci.org/physycom/nmea_to_json.png?branch=master" width="90" height="20" alt="Build Status"> 
</div>
</a>
<a href="https://ci.appveyor.com/project/cenit/nmea-to-json"> 
<div class="image">
<img src="https://ci.appveyor.com/api/projects/status/4kx364yve1g7ceoi?svg=true" width="90" height="20" alt="Build Status"> 
</div>
</a>

### Purpose
This tool has been written in order to convert NMEA data into our standard .json format for GNSS coordinates

### Installation
**CMake**, **boost** and a **C++11** compatible compiler are required. To build the executable, clone the repo and then type  
```
mkdir build ; cd build ; cmake .. ; cmake --build .
```
With CMake you can also deploy projects for the most common IDEs.  
Contains [jsoncons](https://github.com/danielaparker/jsoncons) as a git submodule.

### Usage
```
nmea_to_json.exe -i input.txt -o output.json -f [output style, 'a' (no quotes) for array or 'o' for object]
```
where `input.txt` must be an existing NMEA ASCII-encoded file while `output.json` is the name of the output file.

The optional -f specifies the style of the output .json file. If omitted, the object-style will be used.


### Input Samples
###### Sample #1
```
$GPRMC,135221.000,V,4429.94755,N,01121.18915,E,0.0,-18.4,300615,,E*40
$GPRMC,135222.000,V,4429.94755,N,01121.18915,E,0.0,-18.4,300615,,E*43
```
###### Sample #2
```
$GNRMC,140502.20,A,4430.12967,N,01121.89356,E,7.201,56.95,300615,,,A*40
$GNRMC,140503.00,A,4430.12729,N,01121.89572,E,9.035,58.54,300615,,,A*4F
```
###### Sample #3
```
$GNGGA,135148.00,4429.97640,N,01121.21051,E,1,08,1.63,41.9,M,45.5,M,,*7F
$GNGGA,135148.20,4429.97639,N,01121.21050,E,1,08,1.63,41.9,M,45.5,M,,*72
```

### Output Sample
Object-style:
```
{
    "gps_record_0000001":
    {
        "lat":44.5021611,
        "lon":11.3648926
    },
    "gps_record_0000002":
    {
        "lat":44.5021381,
        "lon":11.3648768
    }
}
```
array-style:
```
[
    {
        "lat":44.5021611,
        "lon":11.3648926
    },
    {
        "lat":44.5021381,
        "lon":11.3648768
    }
]
```

More details about file formats is available [here](https://github.com/physycom/file_format_specifications/blob/master/formati_file.md).
