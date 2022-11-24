// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "AntennaReport_Reader.hpp"

#include <iostream>
#include <iomanip>

AntennaReport_Reader::AntennaReport_Reader() : report(nullptr), time_start(0), time_stop(0) {}
AntennaReport_Reader::~AntennaReport_Reader() {}

bool AntennaReport_Reader::Read(const std::string& fileName, AntennaViolations* violations) {
  std::cout << "====== Reading antenna violations report file ======" << std::endl;
  time_start = std::clock();
  f.open(fileName, std::ios::in);
  if (!f.is_open()) {
    std::cerr << "__err__ : Can't open given input antenna report file '" << fileName << "'" << std::endl;
    return false;
  }
  if (!violations) {
    f.close();
    return false;
  }
  report = violations;

  //char buf[256];
  std::string buf;
  while (!f.eof()) {
    std::getline(f, buf);
    if(buf.substr(0, 3) == "Net")
      if (!ReadNet(buf)) {
        f.close();
        return false;
      }
  }
  time_stop = std::clock();
  return true;
}

void AntennaReport_Reader::PrintStatistics() {
  for (size_t i = 0; i < (* report).size(); ++i) {
    std::cout << std::setw(4) << i + 1 << " Net '" << (* report)[i].net.netName << "' (layer " << (*report)[i].net.layerName << ")"
              << " for device " << (*report)[i].net.instanceName << "[::" << (*report)[i].net.deviceName << "]."
              << (*report)[i].net.pinName << std::endl;
  }
  std::cout << "Antenna report read in " << (time_stop - time_start) << " msec" << std::endl << std::endl;
}

bool AntennaReport_Reader::ReadNet(std::string &line) {
  line.erase(0, 4);
  AntennaViolation  violation;

  violation.net.netName = line;
  
  std::string buf;

  std::getline(f, buf);

  //*
  while (!buf.empty()) {
    if (!ReadConnection(buf, violation))
      return false;
    std::streampos lastFilePosition = f.tellg();
    std::getline(f, buf);
    if (buf.substr(0, 3) == "Net") {
      f.seekg(lastFilePosition);
      return true;
    }
    if (buf[0] == '[') {
      f.seekg(lastFilePosition);
      return true;
    }
  }
  //*/
  return true;
}

bool AntennaReport_Reader::ReadConnection(std::string& line, AntennaViolation& av) {
  size_t pos = std::string::npos;
  pos = line.find_first_not_of(" \t");
  if (pos != std::string::npos)
    line.erase(0, pos);

  pos = line.find('/');
  if (pos == std::string::npos) {
    std::cerr << "_err_ [ANT]: Can't find INSTANCE/pin declaration for net " << av.net.netName << std::endl;
    return false;
  }
  av.net.instanceName = line.substr(0, pos);
  line.erase(0, pos + 1);

  pos = line.find(' ');
  if (pos == std::string::npos) {
    std::cerr << "_err_ [ANT]: Can't find instance/PIN declaration for net " << av.net.netName << std::endl;
    return false;
  }
  av.net.pinName = line.substr(0, pos);
  line.erase(0, pos + 1);

  line.erase(0, 1);
  line.erase(line.length() - 1, 1);
  av.net.deviceName = line;

  std::string layerName, arType, currVal, ratio, maxVal, type;
  //f >> layerName;
  std::getline(f, layerName);

  std::streampos lastFilePosition;

  while (!layerName.empty()) {
    pos = layerName.find_first_not_of(" \t");
    layerName.erase(0, pos);
    av.net.layerName = layerName;

    std::string buf;
    std::getline(f, buf);
    while (!buf.empty()) {
      //arType
      pos = buf.find_first_not_of(" \t");
      buf.erase(0, pos);
      pos = buf.find_first_of(" \t");
      arType = buf.substr(0, pos);
      buf.erase(0, pos);

      //currVal
      pos = buf.find_first_not_of(" \t");
      buf.erase(0, pos);
      pos = buf.find_first_of(" \t");
      currVal = buf.substr(0, pos);

      if (currVal[currVal.length() - 1] != '*') {
        std::getline(f, buf);
        continue;
      }

      buf.erase(0, pos);

      //ratio
      pos = buf.find_first_not_of(" \t");
      buf.erase(0, pos);
      pos = buf.find_first_of(" \t");
      buf.erase(0, pos);

      //maxVal
      pos = buf.find_first_not_of(" \t");
      buf.erase(0, pos);
      pos = buf.find_first_of(" \t");
      maxVal = buf.substr(0, pos);
      buf.erase(0, pos);

      //type
      pos = buf.find_first_not_of(" \t");
      buf.erase(0, pos);
      pos = buf.find_first_of(" \t");
      type = buf.substr(0, pos);
      buf.erase();

      if (currVal[currVal.length() - 1] == '*') {
        if (arType == "CAR:")
          av.violationType = ViolationType::CAR;
        else
          if (arType == "PAR:")
            av.violationType = ViolationType::PAR;
          else {
            std::cerr << "_err_ [ANT]: Can't antenna ratio type for " << av.net.netName << " (layer: " << av.net.layerName << ")" << std::endl;
            return false;
          }

        currVal.erase(currVal.end() - 1);

        av.value = std::stod(currVal);
        av.maxAllowedValue = std::stod(maxVal);

        if (type == "(Area)")
          av.areaType = AreaType::Area;
        else
          if (type == "(S.Area)")
            av.areaType = AreaType::S_Area;
          else
            if (type == "(C.Area)")
              av.areaType = AreaType::C_Area;
            else
              if (type == "(C.S.Area)")
                av.areaType = AreaType::C_S_Area;
              else {
                std::cerr << "_err_ [ANT]: Can't detect area type for " << av.net.netName << " (layer: " << av.net.layerName << ")" << std::endl;
                return false;
              }
        report->push_back(av);
      }
      std::getline(f, buf);
    }
  
    lastFilePosition = f.tellg();
    std::getline(f, layerName);

    if (layerName.empty())
      return true;

    if (layerName[layerName.size() - 1] == ')') {
      f.seekg(lastFilePosition);
      return true;
    }

  }


  return true;
}
