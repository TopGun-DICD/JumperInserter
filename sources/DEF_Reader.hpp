// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <string>
#include <fstream>
#include <ctime>

#include "DEF.hpp"

class DEF_Reader {
  std::fstream  f;
  DEF          *def;
  time_t        time_start,
                time_stop;
public:
  DEF_Reader();
 ~DEF_Reader();
public:
  bool Read(const std::string &fileName, DEF *defData);
  void PrintStatistics();
private:
  bool ReadRow();
  bool ReadTrack();
  bool ReadGCellGrid();
  bool ReadVIAs();
  bool ReadComponents();
  bool ReadPins();
  bool ReadSpecialNets();
  bool ReadNets();
};

