// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <string>
#include <fstream>
#include <ctime>

#include "DEF.hpp"

class DEF_Writer {
  std::fstream  f;
  DEF          *def;
  time_t        time_start,
                time_stop;
public:
  DEF_Writer();
 ~DEF_Writer();
public:
  bool Write(const std::string &fileName, DEF *defData);
  void PrintStatistics();
private:
  bool WriteHeader();
  bool WriteRows();
  bool WriteTracks();
  bool WriteGCellGrids();
  bool WriteVIAs();
  bool WriteComponents();
  bool WritePins();
  bool WriteSpecialNets();
  bool WriteNets();
  bool WriteFooter();
};

