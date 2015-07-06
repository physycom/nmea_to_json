#!/bin/bash
CONVERTER="./xml.exe"
for kml in *.kml; do
	out="${kml%.*}.json"
	$CONVERTER -i $kml -o $out
done
