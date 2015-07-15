### Installation
**Make** and a **C++11** compatible compiler are required. Clone the repo and type ``make all`` in your favourite shell.

Uses jsoncons library (https://github.com/danielaparker/jsoncons) and the boost C++ libraries (http://www.boost.org/).

### Usage
```
nmea2json.exe -i input -o output.json -f [output style, 'a' (no quotes) for array or 'o' for object]
```
where *input* must be an existing NMEA ascii-encoded file while *output.json* is the name of the output archive.

The optional -f specifies the style of the output json file (see below for an example). If omitted, the object-style will be used.


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

Brought to you with :heart: by:

&copy; _Physics of Complex Systems Laboratory - Physics and Astronomy Department - University of Bologna_
