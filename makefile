EXE = nmea_to_json.exe


all: dirtree
	$(CXX) -std=c++11 -Isrc/jsoncons/src -o bin/$(EXE) src/nmea_to_json.cpp

dirtree:
	@mkdir -p bin

clean:
	rm -f bin/$(EXE)

cleanall:
	rm -rf bin obj
