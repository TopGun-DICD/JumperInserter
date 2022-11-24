// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "DEF_Reader.hpp"

#include <iostream>

DEF_Reader::DEF_Reader() : def(nullptr), time_start(0), time_stop(0) {}

DEF_Reader::~DEF_Reader() {}

bool DEF_Reader::Read(const std::string &fileName, DEF *defData) {
  std::cout << "====== Reading DEF file ======" << std::endl;
  time_start = std::clock();
  f.open(fileName, std::ios::in);
  if (!f.is_open()) {
    std::cerr << "__err__ : Can't open given input DEF file '" << fileName << "'" << std::endl;
    return false;
  }
  if (!defData) {
    f.close();
    return false;
  }
  def = defData;

  //char buf[256];
  std::string buf;
  while (!f.eof()) {
    f >> buf;
    if (buf == "VERSION") {
      f >> def->version; 
      f >> buf;
      if (buf != ";") {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "DIVIDERCHAR") {
      char sym;
      f >> sym >> def->dividerChar >> sym >> buf;
      if (buf != ";") {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "BUSBITCHARS") {
      f >> buf;
      def->busBitChars[0] = buf[1];
      def->busBitChars[1] = buf[2];
      f >> buf;
      if (buf != ";") {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "DESIGN") {
      f >> def->designName;
      f >> buf;
      if (buf != ";") {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "UNITS") {
      do {
        f >> buf;
        def->units += " " + buf;
      } while (buf != ";");
      continue;
    }
    if (buf == "DIEAREA") {
      int32_t num1, num2 = 0;
      f >> buf;
      while (buf == "(") {
        f >> num1 >> num2;
        def->dieArea.push_back({ num1, num2 });
        f >> buf; //  ')'
        f >> buf;
      }
      continue;
    }
    if (buf == "ROW") {
      if (!ReadRow()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "TRACKS") {
      if (!ReadTrack()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "GCELLGRID") {
      if (!ReadGCellGrid()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "VIAS") {
      if (!ReadVIAs()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "COMPONENTS") {
      if (!ReadComponents()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "PINS") {
      if (!ReadPins()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "SPECIALNETS") {
      if (!ReadSpecialNets()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "NETS") {
      if (!ReadNets()) {
        f.close();
        return false;
      }
      continue;
    }
    if (buf == "END") {
      f >> buf;
      if (buf == "DESIGN")
        break;
      std::cerr << "_err_: Top-level END was met but not for DESIGN (\'" << buf << "\' was specified instead)" << std::endl;
      return false;
    }
    std::cerr << "_err_ : Unsupported DEF statement : '" << buf << "'. Ignoring (until ';')" << std::endl;
    while (buf != ";" && !f.eof())
      f >> buf;
  }
  time_stop = std::clock();
  return true;
}

void DEF_Reader::PrintStatistics() {
  // Print statistics
  bool isRectDie = def->dieArea.size() == 2 ? true : false;
  std::cout << "DEF file statistics (in read items):" << std::endl;
  if (isRectDie)
    std::cout << "  - Die is rect    : true" << std::endl << "  - Die area       : " << (def->dieArea[1].x - def->dieArea[0].x) * (def->dieArea[1].y - def->dieArea[0].y) << std::endl;
  else
    std::cout << "  - Die is rect    : false" << std::endl;
  std::cout << "  - Rows           : " << def->rows.size() << std::endl;
  std::cout << "  - Tracks         : " << def->tracks.size() << std::endl;
  std::cout << "  - VIAs specified : " << def->vias.size() << std::endl;
  std::cout << "  - Components     : " << def->components.size() << std::endl;
  //std::cout << "  - Special nets   : " << def->components.size() << std::endl;
  std::cout << "  - Nets           : " << def->nets.size() << std::endl;
  std::cout << "DEF file read in " << (time_stop - time_start) << " msec" << std::endl << std::endl;
}

Orientation TranslateOrientation(const std::string &o) {
  switch (o[0]) {
    case 'N': return Orientation::North;
    case 'S': return Orientation::South;
    case 'E': return Orientation::East;
    case 'W': return Orientation::West;
    case 'F':
      switch (o[1]) {
        case 'N': return Orientation::FlippedNorth;
        case 'S': return Orientation::FlippedSouth;
        case 'E': return Orientation::FlippedEast;
        case 'W': return Orientation::FlippedWest;
      }
    default:  return Orientation::North;
  }
}

NetUse TranslateNetUse(const std::string& nu) {
  if (nu == "ANALOG")
    return NetUse::analog;
  else
    if (nu == "CLOCK")
      return  NetUse::clock;
    else
      if (nu == "GROUND")
        return  NetUse::ground;
      else
        if (nu == "POWER")
          return  NetUse::power;
        else
          if (nu == "RESET")
            return  NetUse::reset;
          else
            if (nu == "SCAN")
              return  NetUse::scan;
            else
              if (nu == "SIGNAL")
                return  NetUse::signal;
              else
                if (nu == "TIEOFF")
                  return  NetUse::tieoff;
  return NetUse::undefined;
}

bool DEF_Reader::ReadRow() {
  Row row;
  std::string buf;
  f >> row.rowName >> row.siteName >> row.origXY.x >> row.origXY.y;
  f >> buf;
  row.orientation = TranslateOrientation(buf);
  f >> buf;
  if (buf == ";")
    return true;
  if (buf != "DO")
    return false;
  f >> row.numXY.x;
  f >> buf;
  if (buf != "BY")
    return false;
  f >> row.numXY.y;
  f >> buf;
  if (buf != "STEP")
    return false;
  f >> row.stepXY.x;
  f >> row.stepXY.y;
  f >> buf;
  if (buf != ";")
    return false;
  def->rows.push_back(row);
  return true;
}

bool DEF_Reader::ReadTrack() {
  //std::cout << "Skipping track info" << std::endl;
  Track track;
  std::string buf;
  //do {
    f >> buf;
    if (buf == "X")
      track.direction = TrackDirection::X;
    else
      track.direction = TrackDirection::Y;
    f >> track.start;
    f >> buf;
    f >> track.numTracks;
    f >> buf;
    f >> track.step;
    f >> buf;
    if (buf != "LAYER") {
      std::cerr << "_err_ [DEF]: keyword 'LAYER' expected!" << std::endl;
      return false;
    }
    f >> track.layerName;
    def->tracks.push_back(track);
    f >> buf;
  //} while (buf != ";");
  return true;
}

bool DEF_Reader::ReadGCellGrid() {
  //std::cout << "Skipping gcellgrid info" << std::endl;
  std::string buf;
  std::string gcg;
  do {
    f >> buf;
    gcg += " " + buf;
  } while (buf != ";");
  def->gCellGrids.push_back(gcg);
  return true;
}

bool DEF_Reader::ReadVIAs() {
  int N = 0;
  std::string buf;
  f >> N >> buf;
  def->vias.reserve(N);
  for (size_t i = 0; i < N; ++i) {
    VIA via;
    via.givenRowCol = false;
    f >> buf >> via.name >> buf >> buf >> via.rule >> buf >> buf >> via.cutSize.x >> via.cutSize.y >> buf >> buf >> via.met1 >> via.via >> via.met2 >> buf >> buf
      >> via.cutSpacing.x >> via.cutSpacing.y >> buf >> buf >> via.enclosure.leftBottom.x >> via.enclosure.leftBottom.y >> via.enclosure.rightTop.x >> via.enclosure.rightTop.y;
    f >> buf;
    if (buf == ";") {
      def->vias.push_back(via);
      continue;
    }
    if (buf != "+") {
      std::cerr << "_err_ [DEF]: unexpected token: '" << buf << "' while reading via #" << i << std::endl;
      return false;
    }
    f >> buf;
    if (buf != "ROWCOL") {
      while (buf != ";")
        f >> buf;
      return true;
    }
    via.givenRowCol = true;
    f >> via.rowCol.x >> via.rowCol.y;
    f >> buf;
    if (buf != ";")
      while (buf != ";")
        f >> buf;
    def->vias.push_back(via);
  }
  f >> buf;
  if (buf != "END") {
    std::cerr << "_err_ [DEF]: expected token 'END' after reading " << N << " VIAs but token '" << buf << "' was found"<< std::endl;
    return false;
  }
  f >> buf;
  if (buf != "VIAS") {
    std::cerr << "_err_ [DEF]: expected token 'END VIAS' after reading " << N << " VIAs but token 'END " << buf << "' was found" << std::endl;
    return false;
  }
  return true;
}

bool DEF_Reader::ReadComponents() {
  int32_t num;
  std::string buf;
  f >> num;
  def->components.reserve(num);
  f >> buf; // ';'

  for (size_t i = 0; i < def->components.capacity(); ++i) {
    Component c;
    f >> buf;   // '-'
    f >> c.name;
    f >> c.cellName;
    f >> buf;   // '+'
    f >> buf;   // keyword
    while (buf != ";") {
      if (buf == "PLACED") {
        f >> buf;   // '('
        f >> c.xy.x >> c.xy.y;
        f >> buf;   // ')'
        f >> buf;
        c.orientation = TranslateOrientation(buf);
        c.placement = CellPlacement::placed;
        f >> buf;
        continue;
      }
      if (buf == "FIXED") {
        f >> buf;   // '('
        f >> c.xy.x >> c.xy.y;
        f >> buf;   // ')'
        f >> buf;
        c.orientation = TranslateOrientation(buf);
        c.placement = CellPlacement::fixed;
        f >> buf;
        continue;
      }
      if (buf == "COVER") {
        f >> buf;   // '('
        f >> c.xy.x >> c.xy.y;
        f >> buf;   // ')'
        f >> buf;
        c.orientation = TranslateOrientation(buf);
        c.placement = CellPlacement::cover;
        f >> buf;
        continue;
      }
      if (buf == "UNPLACED")
        continue;
      if (buf == "SOURCE") {
        f >> buf;
        if (buf == "NETLIST")
          c.source = CellSource::netlist;
        else
          if (buf == "DIST")
            c.source = CellSource::dist;
          else
            if (buf == "USER")
              c.source = CellSource::user;
            else
              if (buf == "TIMING")
                c.source = CellSource::timing;
              else
                return false;
        continue;
      }
      f >> buf;
    }
    def->components.push_back(c);
  }
  f >> buf;
  f >> buf;
  return true;
}

bool DEF_Reader::ReadPins() {
  std::cout << "Skipping PINS section" << std::endl;
  /*
  std::string buf;
  do {
    f >> buf;
  } while (buf != "END");
  f >> buf;
  if (buf != "PINS")
    return false;
    */
  f >> def->nPins;
  std::string line;
  f >> line;
  do {
    std::getline(f, line);
    def->pins.push_back(line);
  } while (line != "END PINS");
  return true;
}

bool DEF_Reader::ReadSpecialNets() {
  std::cout << "Skipping SPECIALNETS section" << std::endl;
  /*std::string buf;
  do {
    f >> buf;
  } while (buf != "END");
  f >> buf;
  if (buf != "SPECIALNETS")
    return false;*/

  f >> def->nSpecialNets;
  std::string line;
  f >> line;
  do {
    std::getline(f, line);
    def->specialNets.push_back(line);
  } while (line != "END SPECIALNETS");
  return true;
}

bool DEF_Reader::ReadNets() {
  int32_t num;
  std::string buf;
  f >> num;
  def->nets.reserve(num);
  f >> buf; // ';'

  for (size_t i = 0; i < def->nets.capacity(); ++i) {
    Net net;
    f >> buf; // '-'
    f >> net.name;
    f >> buf; // '('
    do {
      NetConnection nc;
      f >> nc.deviceName >> nc.pinName;
      if (nc.deviceName == "PIN")
        nc.type = NetType::pin;
      net.connections.push_back(nc);
      f >> buf; // ')'
      f >> buf; // '? ('
    } while (buf == "(");
    while (buf != ";") {
      if (buf == "+")
        f >> buf;
      if (buf == "USE") {
        f >> buf;
        net.use = TranslateNetUse(buf);
        if (net.use == NetUse::undefined) {
          std::cerr << "_err_ [NETS]: (\'" << net.name << "\') undefined USE value: \'" << buf << "\'" << std::endl;
          return false;
        }
        f >> buf;
        continue;
      }
      if (buf == "ROUTED") {
        while (buf != "+" && buf != ";") {
          RoutedMetal me;
          f >> buf;
          if (buf == "NEW")
            continue;
          me.name = buf;
          f >> buf; // '('
          f >> me.first.x >> me.first.y >> buf;
          if (buf != ")") {
            if (buf != "0") {
              std::cerr << "_err_ [NETS]: (\'" << net.name << "\') unclosed 1st point for metal \'" << me.name << "\'[" << net.routingPoints.size() << "]: \'" << buf << "\' when ')' expected" << std::endl;
              return false;
            }
            f >> buf;
          }
          f >> buf;
          if (buf == "(") {
            f >> buf;
            if (buf == "*")
              me.last.leftBottom.x = me.first.x;
            else
              me.last.leftBottom.x = std::stoi(buf);
            f >> buf;
            if (buf == "*")
              me.last.leftBottom.y = me.first.y;
            else
              me.last.leftBottom.y = std::stoi(buf);
            f >> buf; // ')'
            me.mode = LastPointMode::point;
          } 
          else
            if (buf == "RECT") {
              f >> buf; // '('
              f >> me.last.leftBottom.x >> me.last.leftBottom.y >> me.last.rightTop.x >> me.last.rightTop.y;
              f >> buf; // ')'
              me.mode = LastPointMode::rect;
            }
            else {
              me.viaName = buf;
              me.mode = LastPointMode::via;
            }
          net.routingPoints.push_back(me);
          f >> buf;
        }
      }
    }
    def->nets.push_back(net);
  }
  f >> buf;
  if (buf != "END") {
    std::cerr << "_err_ [NETS]: \' Keyword 'END' expected after successfully readed " << def->nets.size() << " nets" << std::endl;
    return false;
  }
  f >> buf;
  if (buf != "NETS") {
    std::cerr << "_err_ [NETS]: \' Keyword 'NETS' expected after keyword 'END' when successfully readed " << def->nets.size() << " nets" << std::endl;
    return false;
  }
  return true;
}


