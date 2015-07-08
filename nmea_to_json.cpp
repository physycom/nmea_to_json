#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "jsoncons/json.hpp"

using namespace jsoncons;
using namespace boost::algorithm;


#define MAJOR_VERSION          2
#define MINOR_VERSION          0

/***********************************************************************
 * ((Read_NMEA)) Reads lat-lon-alt data from NMEA file                 *
 ***********************************************************************/

int checksum(const char *s) {
    int c = 0;
    while(*s)  c ^= *s++;
    return c;
}


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
  input_file.open(input_name.c_str(), std::ios::binary);
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
  std::vector<std::vector <std::string>> file_tokens, file_sentences;
  int gps_record_counter = 0;
  std::stringstream ss;
  std::string record_name;
  json outjson;

  // try with standard GPRMC
  std::cout << "Looking for $GPRMC NMEA data" << std::endl;
  std::string line;
  while (std::getline(input_file, line, '$')) {
    std::vector<std::string> tokens, sentences;
    tokens.clear();
    sentences.clear();
    ss.str("\0");
    ss.seekp(0, std::ios::beg);

    split(tokens, line, is_any_of(",*\n"), token_compress_off);
    split(sentences, line, is_any_of("*"), token_compress_off);

    if ( tokens.size() > 0 ){
      if (tokens[0] == "GPRMC") {
        //if ( tokens[2] == "V") continue;            // "Void" code -> data rejected
        if (tokens[2] == "A") {                       // "Accept" code
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

          ss << std::hex << checksum(sentences[0].c_str() );
          std::transform(tokens[12].begin(), tokens[12].end(), tokens[12].begin(), ::tolower);
          if ( tokens[12].compare( ss.str() ) ){
            gps_record_counter++;
            json ijson;
            ijson["description"] = sentences[0];
            ijson["lat"] = lat;
            ijson["lon"] = lon;

            ss.str("\0");
            ss.seekp(0, std::ios::beg);
            ss << std::setfill('0') << std::setw(7) << gps_record_counter;
            record_name = "gps_record_" + ss.str();
            outjson[record_name] = ijson;
          } 
        }
      }
      file_tokens.push_back(tokens);
      file_sentences.push_back(sentences);
    }
  }
  if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GPRMC" << std::endl;

  // fallback to uBlox GNRMC
  if (!gps_record_counter) {
    std::cout << "No valid $GPRMC NMEA data, falling back to $GNRMC" << std::endl;
    for(size_t i=0; i < file_sentences.size(); i++){
      ss.str("\0");
      ss.seekp(0, std::ios::beg);

      if (file_tokens[i][0] == "GNRMC") {
        if (file_tokens[i][2] == "A") {
          double lat, lon;
          lat = 1e-2*atof(file_tokens[i][3].c_str());
          lon = 1e-2*atof(file_tokens[i][5].c_str());

          json ijson;
          ijson["description"] = file_sentences[i][0];
          ijson["lat"] = lat;
          ijson["lon"] = lon;

          ss << std::hex << checksum(file_sentences[i][0].c_str() );
          std::transform(file_tokens[i][13].begin(), file_tokens[i][13].end(), file_tokens[i][13].begin(), ::tolower);
          if ( file_tokens[i][13].compare( ss.str() ) ){
            gps_record_counter++;
            json ijson;
            ijson["description"] = file_sentences[i][0];
            ijson["lat"] = lat;
            ijson["lon"] = lon;

            ss.str("\0");
            ss.seekp(0, std::ios::beg);
            ss << std::setfill('0') << std::setw(7) << gps_record_counter;
            record_name = "gps_record_" + ss.str();
            outjson[record_name] = ijson;
          } 
        }
      }
    }
    if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GNRMC" << std::endl;
  }

  if(!gps_record_counter){
    std::cout << "No valid $GNRMC NMEA data, falling back to $GNGGA" << std::endl;

    for(size_t i=0; i < file_sentences.size(); i++){
      ss.str("\0");
      ss.seekp(0, std::ios::beg);

      if (file_tokens[i][0] == "GNGGA") {
        double lat, lon;

        int lati, loni;
        lati = atoi(file_tokens[i][2].c_str())/100;
        loni = atoi(file_tokens[i][4].c_str())/100;
        lat = atof(file_tokens[i][2].c_str())/100.0 - (double)lati;
        lat /= 60.0;
        lat += (double) lati;
        lon = atof(file_tokens[i][4].c_str())/100.0 - (double)loni;
        lon /= 60.0;
        lon += (double) loni;

        ss << std::hex << checksum(file_sentences[i][0].c_str() );
        std::transform(file_tokens[i][15].begin(), file_tokens[i][15].end(), file_tokens[i][15].begin(), ::tolower);
        if ( file_tokens[i][15].compare( ss.str() ) ){
          gps_record_counter++;
          json ijson;
          ijson["description"] = file_sentences[i][0];
          ijson["lat"] = lat;
          ijson["lon"] = lon;

          ss.str("\0");
          ss.seekp(0, std::ios::beg);
          ss << std::dec << std::setfill('0') << std::setw(7) << gps_record_counter;
          record_name = "gps_record_" + ss.str();
          outjson[record_name] = ijson;
        } 
      }
    }
    if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GNGGA" << std::endl;
  }

  if (gps_record_counter) {
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    if (!output_file.is_open()) {
      std::cout << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cout << "Type q to quit or any other character to have a fallback output on stdout." << std::endl;
      char q;
      std::cin >> q;
      if (q == 'q') exit(333);
      else std::cout << pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << pretty_print(outjson) << std::endl;
    output_file.close();
  }
  else std::cout << "No valid NMEA $--RMC nor $GNGGA data found" << std::endl;

  input_file.close();

  return 0;
}

