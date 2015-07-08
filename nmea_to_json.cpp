#include <iostream>
#include <string>
#include <fstream>
#include "jsoncons/json.hpp"

using namespace jsoncons;

#define MAJOR_VERSION          1
#define MINOR_VERSION          1


/***********************************************************************
 * ((Read_NMEA)) Reads lat-lon-alt data from NMEA file                 *
 ***********************************************************************/

int main(int argc, char** argv)
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
  else {
    std::cout << "No flags specified. Read usage and relaunch properly." << std::endl;
    exit(111);
  }

  // Safety checks for file manipulations
  std::ifstream input_file;
  if (input_name == ""){
    std::cout << "INPUT file missing. Quitting..." << std::endl;
    exit(2);
  }
  input_file.open(input_name.c_str());
  if (!input_file.is_open()) {
    std::cout << "FAILED: Input file " << input_name << " could not be opened." << std::endl;
    std::cout << "Press a key to close.\n"; 
    std::cin.get();
    exit(22);
  }
  else std::cout << "SUCCESS: file " << input_name << " opened!" << std::endl;

  if (output_name.size() > 5){
    if (output_name.substr(output_name.size() - 5, 5) != ".json"){
      std::cout << output_name << " is not a valid .json file. Quitting..." << std::endl;
      exit(3);
    }
  }
  else {
    std::cout << output_name << " is not a valid .json file. Quitting..." << std::endl;
    exit(33);
  }

  // NMEA parser
  std::vector<std::string> tokens;
  char * token;
  std::string line;
  int gps_record_counter = 0;
  std::stringstream ss;
  std::string record_name;
  json outjson;

  // try with standard GPRMC
  std::cout << "Looking for $GPRMC NMEA data" << std::endl;
  while (std::getline(input_file, line, '$')) {
    tokens.clear();
    ss.str("\0");
    ss.seekp(0, std::ios::beg);

    token = std::strtok((char *)line.c_str(), ",");
    while (token != NULL){
      tokens.push_back(token);
      token = std::strtok(NULL, ",");
    }

    if (tokens[0] == "GPRMC") {
      //if ( tokens[2] == "V") continue;            // "Navigation receiver warning" code -> data rejected
      if (tokens[2] == "A") {
        gps_record_counter++;

        double lat, lon;
        // non valid for lat/lon<10 and for lon>99
        //lat = stod(tokens[3].substr(0,2)) + stod(tokens[3].substr(2))/60.0;
        //lon = stod(tokens[5].substr(0,2)) + stod(tokens[5].substr(2))/60.0;

        int lati, loni;
        lati = atoi(tokens[3].c_str())/100;
        loni = atoi(tokens[5].c_str())/100;
        lat = atof(tokens[3].c_str())/100.0 - (double)lati;
        lat /= 60.0;
        lat += (double) lati;
        lon = atof(tokens[5].c_str())/100.0 - (double)loni;
        lon /= 60.0;
        lon += (double) loni;

        json ijson;
        ijson["description"] = line;
        ijson["lat"] = lat;
        ijson["lon"] = lon;

        ss << std::setfill('0') << std::setw(7) << gps_record_counter;
        record_name = "gps_record_" + ss.str();
        outjson[record_name] = ijson;
      }
    }
  }

  // fallback to uBlox GNRMC
  if (!gps_record_counter) {
    std::cout << "No valid $GPRMC NMEA data, fallig back to $GNRMC" << std::endl;
    input_file.clear();
    input_file.seekg(0, std::ios::beg);
    while (std::getline(input_file, line, '$')) {
      tokens.clear();
      ss.str("\0");
      ss.seekp(0, std::ios::beg);

      token = std::strtok((char *)line.c_str(), ",");
      while (token != NULL){
        tokens.push_back(token);
        token = std::strtok(NULL, ",");
      }

      if (tokens[0] == "GNRMC") {
        //if ( tokens[2] == "V") continue;            // "Navigation receiver warning" code -> data rejected
        if (tokens[2] == "A") {
          gps_record_counter++;
          double lat, lon;
          lat = 1e-2*atof(tokens[3].c_str());
          lon = 1e-2*atof(tokens[5].c_str());

          json ijson;
          ijson["description"] = line;
          ijson["lat"] = lat;
          ijson["lon"] = lon;

          ss << std::setfill('0') << std::setw(7) << gps_record_counter;
          record_name = "gps_record_" + ss.str();
          outjson[record_name] = ijson;
        }
      }
    }
  }

  if (gps_record_counter) {
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    if (!output_file.is_open()) {
      std::cout << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cout << "Press q to quit, any other key to have a fallback output on stdout." << std::endl;
      char q;
      std::cin.get(&q,1);
      if (q == 'q') exit(333);
      else std::cout << pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << pretty_print(outjson) << std::endl;
    output_file.close();
  }
  else std::cout << "No valid NMEA $--RMC data found" << std::endl;

  input_file.close();

  return 0;
}

