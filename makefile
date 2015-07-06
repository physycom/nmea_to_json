EXE = nmea_to_json.exe

all:
	g++ -std=c++11 -o $(EXE) -I. nmea_to_json.cpp -fpermissive
