#!/bin/bash
CONVERTER="./nmea_to_json.exe"
for ubx in *.ubx; do
	out="${ubx%.*}.json"
	$CONVERTER -i $ubx -o $out
done
