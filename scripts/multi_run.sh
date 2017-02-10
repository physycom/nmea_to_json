#!/bin/bash
CONVERTER="./nmea2json.exe"
for ubx in *.ubx; do
	out="${ubx%.*}.json"
	$CONVERTER -i $ubx -o $out
done
