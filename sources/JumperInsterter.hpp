// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <ctime>

#include "DEF.hpp"
#include "AntennaViolation.hpp"

class JumperInserter {
  size_t  foundNetsNumber;
  time_t  time_start,
          time_stop;
  size_t  violationsFixed,
          violationsLeaved;
public:
  JumperInserter();
public:
  bool ProcessViolatedNets(DEF &def, AntennaViolations &violations, bool doEcho = true);
  void PrintStatistics();
private:
  bool ProcessViolation(DEF& def, AntennaViolation& violation, bool doEcho);
};
