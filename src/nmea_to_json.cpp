/* Copyright 2015 - Alessandro Fabbri, Stefano Sinigardi, Giulia Venturi */

/***************************************************************************
This file is part of nmea_to_json.

nmea_to_json is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

nmea_to_json is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with nmea_to_json. If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include <iostream>
#include <string>

#include <fstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include "jsoncons/json.hpp"

using namespace jsoncons;
using namespace boost::algorithm;

#define MAJOR_VERSION          3
#define MINOR_VERSION          6

#define DELTA_SEC_EPOCH           946684800                              // seconds from 1/1/1970:00:00:00 to 31/12/1999:23:59:59
#define SEC_IN_HOUR               3600
#define DELTA_H_UTC_TO_ROME       1

int checksum(const char *s) {
  int c = 0;
  while (*s)  c ^= *s++;
  return c;
}

void usage(char * progname) {
  std::cerr << "Usage: " << progname << " -i [input] -o [output.json] -f [output format specifier] -d [date]" << std::endl;
  std::cerr << "\t- [input] NMEA encoded ascii file to parse" << std::endl;
  std::cerr << "\t- [output.json] json location to store parsed file" << std::endl;
  std::cerr << "\t- [format specifier optional] use 'a' (without quotes) for array json, 'o' for object json" << std::endl;
  std::cerr << "\t- [date] enter input date in the format YYYYMMDD" << std::endl;
}

int main(int argc, char** argv)
{
  std::cout << "nmea_to_json v" << MAJOR_VERSION << "." << MINOR_VERSION << std::endl;

  // Parsing command line
  std::string input_name, output_name, outjson_type{}, date;
  if (argc > 2) { /* Parse arguments, if there are arguments supplied */
    for (int i = 1; i < argc; i++) {
      if ((argv[i][0] == '-') || (argv[i][0] == '/')) {       // switches or options...
        switch (tolower(argv[i][1])) {
        case 'i':
          input_name = argv[++i];
          break;
        case 'o':
          output_name = argv[++i];
          break;
        case 'f':
          outjson_type = argv[++i];
          break;
        case 'd':
          date = argv[++i];
          break;
        default:    // no match...
          std::cerr << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
          usage(argv[0]);
          exit(-1);
        }
      }
      else {
        std::cerr << "Flag \"" << argv[i] << "\" not recognized. Quitting..." << std::endl;
        usage(argv[0]);
        exit(-2);
      }
    }
  }
  else {
    std::cerr << "ERROR : No flags specified." << std::endl;
    usage(argv[0]);
    exit(-3);
  }

  // Setting date in struct tm
  struct tm tm_time;
  std::vector<std::string> datev;
  datev.push_back(date.substr(0, 4));
  datev.push_back(date.substr(4, 2));
  datev.push_back(date.substr(6, 2));
  tm_time.tm_year = atoi(datev[0].c_str()) - 1900;
  tm_time.tm_mon = atoi(datev[1].c_str()) - 1;
  tm_time.tm_mday = atoi(datev[2].c_str());

  // Safety checks for file manipulations
  std::ifstream input_file;
  if (input_name == "") {
    std::cerr << "INPUT file missing. Quitting..." << std::endl;
    exit(-4);
  }
  input_file.open(input_name.c_str(), std::ios::binary);
  if (!input_file.is_open()) {
    std::cerr << "FAILED: Input file " << input_name << " could not be opened." << std::endl;
    exit(-5);
  }
  else std::cout << "SUCCESS: file " << input_name << " opened!" << std::endl;

  if (output_name.size() > 5) {
    if (output_name.substr(output_name.size() - 5, 5) != ".json") {
      std::cerr << output_name << " is not a valid .json file. Quitting..." << std::endl;
      exit(-6);
    }
  }
  else {
    std::cerr << output_name << " is not a valid .json file. Quitting..." << std::endl;
    exit(-7);
  }
  if (date == "") {
    std::cerr << "No DATE specified. Quitting..." << std::endl;
    exit(-8);
  }

  // NMEA parser
  std::vector<std::vector <std::string>> file_tokens, file_sentences;
  int gps_record_counter = 0;
  std::stringstream ss;
  std::string record_name;
  int h, m, s, nano;

  json outjson;
  // decide output type
  if (outjson_type == "a") //array
    outjson = jsoncons::json::array();
  else if (outjson_type == "o" || outjson_type == "") {} //object or omitted
  else {
    std::cerr << "Output type not recognized. Quitting..." << std::endl;
    exit(-9);
  }

  // try with standard GPRMC
  // sample string
  // $GPRMC,135221.000,V,4429.94755,N,01121.18915,E,0.0,-18.4,300615,,E*40
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

    if (tokens.size() > 0) {
      if (tokens[0] == "GPRMC") {
        //if ( tokens[2] == "V") continue;            // "Void" code -> data rejected
        if (tokens[2] == "A") {                       // "Accept" code
          double lat, lon, timestamp;
          // non valid for lat/lon<10 and for lon>99
          //lat = stod(tokens[3].substr(0,2)) + stod(tokens[3].substr(2))/60.0;
          //lon = stod(tokens[5].substr(0,2)) + stod(tokens[5].substr(2))/60.0;

          int lati, loni;
          lati = atoi(tokens[3].c_str()) / 100;
          loni = atoi(tokens[5].c_str()) / 100;
          lat = atof(tokens[3].c_str()) / 100.0 - (double)lati;
          lat /= 60.0 / 100.0;
          lat += (double)lati;
          lon = atof(tokens[5].c_str()) / 100.0 - (double)loni;
          lon /= 60.0 / 100.0;
          lon += (double)loni;

          timestamp = atof(tokens[1].c_str());
          h = int(timestamp / 1e4);
          m = int((timestamp - h*1e4) / 1e2);
          s = int((timestamp - h*1e4 - m*1e2));
          nano = int((timestamp - h*1e4 - m*1e2 - s)*1e2);

          tm_time.tm_hour = h;
          tm_time.tm_min = m;
          tm_time.tm_sec = s;

#ifdef __GNUC__
          time_t timestamp_int = mktime(&tm_time) + 2 * SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#else
          time_t timestamp_int = mktime(&tm_time) + SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#endif
          timestamp = timestamp_int + nano / 1e2;

          ss << std::hex << checksum(sentences[0].c_str());
          std::transform(tokens[12].begin(), tokens[12].end(), tokens[12].begin(), ::tolower);
          if (tokens[12].compare(ss.str())) {
            gps_record_counter++;
            json ijson;
            ijson["description"] = sentences[0];
            ijson["lat"] = lat;
            ijson["lon"] = lon;
            ijson["timestamp"] = timestamp;

            ss.str("\0");
            ss.seekp(0, std::ios::beg);
            ss << std::setfill('0') << std::setw(7) << gps_record_counter;
            record_name = "gps_record_" + ss.str();

            if (outjson.is_array()) outjson.add(ijson);
            else outjson[record_name] = ijson;
          }
        }
      }
      file_tokens.push_back(tokens);
      file_sentences.push_back(sentences);
    }
  }
  if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GPRMC" << std::endl;

  // fallback to uBlox GNRMC
  // sample string
  // $GNRMC,140502.20,A,4430.12967,N,01121.89356,E,7.201,56.95,300615,,,A*40
  if (!gps_record_counter) {
    std::cout << "No valid $GPRMC NMEA data, falling back to $GNRMC" << std::endl;
    for (size_t i = 0; i < file_tokens.size(); i++) {
      ss.str("\0");
      ss.seekp(0, std::ios::beg);

      if (file_tokens[i][0] == "GNRMC") {
        if (file_tokens[i][2] == "A") {
          double lat, lon, timestamp;
          int lati, loni;
          lati = atoi(file_tokens[i][3].c_str()) / 100;
          loni = atoi(file_tokens[i][5].c_str()) / 100;
          lat = atof(file_tokens[i][3].c_str()) / 100.0 - (double)lati;
          lat /= (60.0 / 100.0);
          lat += (double)lati;
          lon = atof(file_tokens[i][5].c_str()) / 100.0 - (double)loni;
          lon /= (60.0 / 100.0);
          lon += (double)loni;

          timestamp = atof(file_tokens[i][1].c_str());
          h = int(timestamp / 1e4);
          m = int((timestamp - h*1e4) / 1e2);
          s = int((timestamp - h*1e4 - m*1e2));
          nano = int((timestamp - h*1e4 - m*1e2 - s)*1e2);

          tm_time.tm_hour = h;
          tm_time.tm_min = m;
          tm_time.tm_sec = s;

#ifdef __GNUC__
          time_t timestamp_int = mktime(&tm_time) + 2 * SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#else
          time_t timestamp_int = mktime(&tm_time) + SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#endif
          timestamp = timestamp_int + nano / 1e2;

          ss << std::hex << checksum(file_sentences[i][0].c_str());
          std::transform(file_tokens[i][13].begin(), file_tokens[i][13].end(), file_tokens[i][13].begin(), ::tolower);
          if (file_tokens[i][13].compare(ss.str())) {
            gps_record_counter++;
            json ijson;
            ijson["description"] = file_sentences[i][0];
            ijson["lat"] = lat;
            ijson["lon"] = lon;
            ijson["timestamp"] = timestamp;

            ss.str("\0");
            ss.seekp(0, std::ios::beg);
            ss << std::setfill('0') << std::setw(7) << gps_record_counter;
            record_name = "gps_record_" + ss.str();

            if (outjson.is_array()) outjson.add(ijson);
            else outjson[record_name] = ijson;
          }
        }
      }
    }
    if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GNRMC" << std::endl;
  }

  // fallback to GNGGA
  // sample string
  // $GNGGA,135148.00,4429.97640,N,01121.21051,E,1,08,1.63,41.9,M,45.5,M,,*7F
  if (!gps_record_counter) {
    std::cout << "No valid $GNRMC NMEA data, falling back to $GNGGA NMEA" << std::endl;
    for (size_t i = 0; i < file_tokens.size(); i++) {
      ss.str("\0");
      ss.seekp(0, std::ios::beg);

      if (file_tokens[i][0] == "GNGGA") {
        double lat, lon, timestamp;

        int lati, loni;
        lati = atoi(file_tokens[i][2].c_str()) / 100;
        loni = atoi(file_tokens[i][4].c_str()) / 100;
        lat = atof(file_tokens[i][2].c_str()) / 100.0 - (double)lati;
        lat /= (60.0 / 100.0);
        lat += (double)lati;
        lon = atof(file_tokens[i][4].c_str()) / 100.0 - (double)loni;
        lon /= (60.0 / 100.0);
        lon += (double)loni;

        timestamp = atof(file_tokens[i][1].c_str());
        h = int(timestamp / 1e4);
        m = int((timestamp - h*1e4) / 1e2);
        s = int((timestamp - h*1e4 - m*1e2));
        nano = int((timestamp - h*1e4 - m*1e2 - s)*1e2);

        tm_time.tm_hour = h;
        tm_time.tm_min = m;
        tm_time.tm_sec = s;

#ifdef __GNUC__
        time_t timestamp_int = mktime(&tm_time) + 2 * SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#else
        time_t timestamp_int = mktime(&tm_time) + SEC_IN_HOUR - DELTA_SEC_EPOCH;        // mktime converts from local time to gmt so we add one hour explicitly
#endif
        timestamp = timestamp_int + nano / 1e2;


        //      std::cout << h << " " << m << "  " << s << "  " << nano  << "   " << timestamp_int <<  std::endl, fatto = true;   // in case of debug

        ss << std::hex << checksum(file_sentences[i][0].c_str());
        std::transform(file_tokens[i][15].begin(), file_tokens[i][15].end(), file_tokens[i][15].begin(), ::tolower);
        if (file_tokens[i][15].compare(ss.str())) {
          gps_record_counter++;
          json ijson;
          ijson["description"] = file_sentences[i][0];
          ijson["lat"] = lat;
          ijson["lon"] = lon;
          ijson["timestamp"] = timestamp;

          std::stringstream human_time;
          human_time << tm_time.tm_mday << "/" << tm_time.tm_mon + 1 << "/" << tm_time.tm_year + 1900 << " "
            << std::setw(2) << std::setfill('0') << tm_time.tm_hour + DELTA_H_UTC_TO_ROME << ":" << std::setw(2) << std::setfill('0') << tm_time.tm_min << ":"
            << std::setw(2) << std::setfill('0') << tm_time.tm_sec << "." << std::setw(2) << std::setfill('0') << nano;
          ijson["date"] = human_time.str();

          ss.str("\0");
          ss.seekp(0, std::ios::beg);
          ss << std::dec << std::setfill('0') << std::setw(7) << gps_record_counter;
          record_name = "gps_record_" + ss.str();

          if (outjson.is_array()) outjson.add(ijson);
          else outjson[record_name] = ijson;
        }
      }
    }
    if (gps_record_counter) std::cout << "Found " << gps_record_counter << " gps record of type $GNGGA" << std::endl;
  }

  if (gps_record_counter) {
    std::ofstream output_file;
    output_file.open(output_name.c_str());
    if (!output_file.is_open()) {
      std::cerr << "FAILED: Output file " << output_name << " could not be opened." << std::endl;
      std::cerr << "Output will be redirected on stdout." << std::endl;
      std::cout << pretty_print(outjson) << std::endl;
    }
    else std::cout << "SUCCESS: file " << output_name << " opened!" << std::endl;
    output_file << pretty_print(outjson) << std::endl;
    output_file.close();
  }
  else std::cerr << "No valid NMEA $--RMC nor $GNGGA data found in " << input_name << std::endl;

  input_file.close();

  return 0;
}

