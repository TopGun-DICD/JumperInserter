// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#pragma once

#include <vector>
#include <string>

// https://github.com/jinwookjungs/lefdef_util/blob/master/src/lefdef/Def.h

struct Point {
  int32_t x,
          y;
};

struct Rect {
  Point   leftBottom,
          rightTop;
};

enum class Orientation : uint8_t {
  North,
  South,
  West,
  East,
  FlippedNorth,
  FlippedSouth,
  FlippedWest,
  FlippedEast,
};

struct Row {
  std::string   rowName,
                siteName;
  Point         origXY;
  Orientation   orientation = Orientation::North;
  Point         numXY = {1, 1},
                stepXY;
};

enum class TrackDirection {
  undefined,
  X,
  Y
};

struct Track {
  TrackDirection  direction;
  uint32_t        start,
                  numTracks,
                  step;
  std::string     layerName;
};

struct GCellGrid {
  uint32_t        startX,
                  startY,
                  numColumns,
                  numRows,
                  vertSpace,
                  horzSpace;
};

enum class CellSource {
  undefined,
  dist,
  netlist,
  timing,
  user,
};

enum class CellPlacement : uint8_t {
  unplaced,
  placed,
  fixed,
  cover
};

struct VIA {
  std::string   name,
                rule;
  std::string   met1,
                via,
                met2;
  Point         cutSize,
                cutSpacing,
                rowCol;
  Rect          enclosure;
  bool          givenRowCol;
};

struct Component {
  std::string     name,
                  cellName;
  Point           xy          = {-1, -1};
  CellPlacement   placement   = CellPlacement::unplaced;
  Orientation     orientation = Orientation::North;
  CellSource      source      = CellSource::undefined;
};

struct Pin {};

struct SpecialNet {};

enum class NetType {
  component,
  pin
};

struct NetConnection {
  NetType         type = NetType::component;
  std::string     deviceName;
  std::string     pinName;
  bool            synthesized = false;
};

enum class NetUse {
  undefined,
  analog,
  clock,
  ground,
  power,
  reset,
  scan,
  signal,
  tieoff
};

enum LastPointMode {
  undefined,
  point,
  rect,
  via
};

struct RoutedMetal {
  std::string   name,
                viaName;
  Point         first = {-1, -1};
  Rect          last = { 0, 0, 0, 0 };
  LastPointMode mode = LastPointMode::undefined;
};

struct Net {
  std::string                 name;
  std::vector<NetConnection>  connections;
  NetUse                      use = NetUse::undefined;
  std::vector<RoutedMetal>    routingPoints;
};

struct DEF {
  // Header
  std::string             version;
  unsigned char           dividerChar;
  unsigned char           busBitChars[2];
  std::string             units;
  std::string             designName;
  std::vector<Point>      dieArea;
  // Data
  std::vector<Row>        rows;
  std::vector<Track>      tracks;
  std::vector<std::string>gCellGrids;
  std::vector<VIA>        vias;
  std::vector<Component>  components;
  //std::vector<Pin>        pins;
  //std::vector<SpecialNet> specialNets;
  std::vector<Net>        nets;
  // dummy data
  int32_t                 nPins;
  std::vector<std::string>pins;
  int32_t                 nSpecialNets;
  std::vector<std::string>specialNets;
};
