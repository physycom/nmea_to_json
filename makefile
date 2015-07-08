EXE = nmea2json.exe

all:
	g++ -std=c++11 -I$(BOOST_INC) -I. -o $(EXE) nmea_to_json.cpp -fpermissive
