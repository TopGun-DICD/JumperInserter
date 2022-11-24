// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <vector>
#include <string>

enum class ReportType {
  undefined,
  net,
  pin
};

enum class ViolationType {
  undefined,
  PAR,
  CAR
};

enum class AreaType {
  undefined,
  Area,
  S_Area,
  C_Area,
  C_S_Area,
};

/*
::netName
Net net13 
  ::instanceName / ::pinName (::deviceName)
  _153_/A1 (sky130_fd_sc_hd__mux2_1)
    ::layerName
    met1
    PAR:  158.09  Ratio:    0.00 (Area)
    PAR:  791.22* Ratio:  400.00 (S.Area)
    CAR:  158.20  Ratio:    0.00 (C.Area)
    CAR:  791.36  Ratio:    0.00 (C.S.Area)

*/
struct ViolationTarget {
  std::string   netName,
                instanceName,
                pinName,
                deviceName,
                layerName;
};

struct AntennaViolation {
  ViolationTarget net;
  ViolationType   violationType;
  AreaType        areaType;
  double          value,
                  maxAllowedValue;
};

typedef std::vector<AntennaViolation> AntennaViolations;
