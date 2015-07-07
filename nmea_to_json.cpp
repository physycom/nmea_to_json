#include <iostream>
#include <string>
#include <fstream>
#include "jsoncons/json.hpp"

using namespace std; 
using namespace jsoncons; 

#define MAJOR_VERSION						1
#define MINOR_VERSION						0

/*
TODO
- fix unità di misura
- districare la questione dell'encoding delle sentence
- il chip UBX non butta NMEA standard

*/


/***********************************************************************************************************/
/* ((Read_NMEA)) Reads lat-lon-alt data from NMEA file                                                     */

int main( int argc, char** argv)
{
// Usage
	std::cout << "Nmea_to_Json v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;
  std::cout << "Usage: " << argv[0] << " -i [input] -o [output.json]" << std::endl;
  std::cout << "\t- [input] NMEA encoded ascii file to parse" << std::endl;
  std::cout << "\t- [output.json] json location to store parsed file" << std::endl;

// Parsing command line
  std::string input_name, output_name;
  if (argc > 2){ /* Parse arguments, if there are arguments supplied */
    for (int i = 1; i < argc; i++){
      if ((argv[i][0] == '-') || (argv[i][0] == '/')){       // switches or options...
        switch (tolower(argv[i][1])){
        case 'i':
          input_name = argv[++i];
          break;
        case 'o':
          output_name = argv[++i];
          break;
        default:    // no match...
          std::cout << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
          exit(1);
        }
      }
      else {
        std::cout << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
        exit(11);
      }
    }
  }
  else { std::cout << "No flags specified. Read usage and relaunch properly." << std::endl; exit(111); }

// Safety checks for file manipulations
  ofstream output_file;
  ifstream input_file;

  if( input_name=="" ){
  	std::cout << "INPUT file missing. Quitting..." << std::endl;
  	exit(2);
  }
  input_file.open(input_name.c_str());  
  if (!input_file.is_open()) {
    cout << "FAILED: Input file " << input_name << " could not be opened." << endl;
    cout << "Hit ENTER to close.\n"; cin.get();
    exit(22);
  }
  else { cout << "SUCCESS: file " << input_name << " opened!\n"; }

  if( output_name.size() > 5 ){
    if( output_name.substr(output_name.size()-5,5)!=".json" ){
      std::cout << output_name << " is not a valid .json file. Quitting..." << std::endl;
      exit(3);
    }
  }
  else{
    std::cout << output_name << " is not a valid .json file. Quitting..." << std::endl;
    exit(33);
  }
  output_file.open(output_name.c_str());
  if (!output_file.is_open()) {
    cout << "FAILED: Output file " << output_name << " could not be opened." << endl;
    cout << "Hit ENTER to close.\n"; cin.get();
    exit(333);
  }
  else { cout << "SUCCESS: file " << output_name << " opened!\n"; }

// NMEA parser
  std::vector<std::string> tokens;
  char * token;
	std::string line;
	int gps_record_counter;
	std::stringstream ss;
	std::string record_name;
	json outjson;
	while( std::getline(input_file, line) ) {
		tokens.clear();
		ss.str("\0");
		ss.seekp(0, std::ios::beg);

		token = std::strtok( (char *) line.c_str(), ",");
		while( token != NULL){
			tokens.push_back(token);
			token = std::strtok( NULL, ",");
		}

		if(tokens[0]=="$GNRMC") {
			if ( tokens[2] == "V") continue;            // "Navigation receiver warning" code -> data rejected
			else if ( tokens[2] == "A") {
				gps_record_counter++;
				double lat, lon, alt;
				lat = 1e-2*atof(tokens[3].c_str());
				lon = 1e-2*atof(tokens[5].c_str());

				json ijson;
				ijson["description"] = line;
				ijson["lat"] = lat;
				ijson["lon"] = lon;

			  ss << setfill('0') << setw(7) << gps_record_counter;
				record_name = "gps_record_"+ss.str();
				outjson[record_name]=ijson;
			}
		}
	}

	output_file << pretty_print(outjson) << std::endl;
	output_file.close(); 
	input_file.close();

	return 0;
}

