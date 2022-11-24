// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <string>
#include <fstream>

#include "AntennaViolation.hpp"

class AntennaReport_Reader {
  std::fstream        f;
  AntennaViolations  *report;
  time_t              time_start,
                      time_stop;
public:
  AntennaReport_Reader();
 ~AntennaReport_Reader();
public:
  bool Read(const std::string& fileName, AntennaViolations *violations);
  void PrintStatistics();
private:
  bool ReadNet(std::string &line);
  bool ReadConnection(std::string &line, AntennaViolation& av);
};
