EXE = nmea_to_json.exe

all:
	$(CXX) -std=c++11 -I. -o $(EXE) nmea_to_json.cpp
