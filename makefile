EXE = nmea_to_json.exe


all:
	$(CXX) -std=c++11 -Isrc -o $(EXE) src/nmea_to_json.cpp
clean:
	rm -rf *.exe *.o x64 Debug Release *.sdf *.ilk *.pdb *.iobj *.obj *.ipdb src/Debug src/Release src/x64
