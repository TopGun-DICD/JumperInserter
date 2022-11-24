// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "DEF_Writer.hpp"

#include <iostream>

DEF_Writer::DEF_Writer() : def(nullptr), time_start(0), time_stop(0) {}

DEF_Writer::~DEF_Writer() {}

bool DEF_Writer::Write(const std::string &fileName, DEF *defData) {
  std::cout << "====== Writing DEF file ======" << std::endl;
  f.open(fileName, std::ios::out);
  if (!f.is_open()) {
    std::cerr << "__err__ : Can't write to given output DEF file '" << fileName << "'" << std::endl;
    return false;
  }
  if (!defData) {
    f.close();
    std::cerr << "__err__ : No DEF data given" << std::endl;
    return false;
  }
  def = defData;
  time_start = std::clock();

  WriteHeader();
  WriteRows();
  WriteTracks();
  WriteGCellGrids();
  WriteVIAs();
  WriteComponents();
  WritePins();
  WriteSpecialNets();
  WriteNets();
  WriteFooter();

  f.close();

  time_stop = std::clock();
  return true;
}

void DEF_Writer::PrintStatistics() {
  std::cout << "DEF file written in " << (time_stop - time_start) << " msec" << std::endl << std::endl;
}

bool DEF_Writer::WriteHeader() {
  f << "VERSION " << def->version << " ;" << std::endl;
  f << "DIVIDERCHAR \"" << def->dividerChar << "\" ;" << std::endl;
  f << "BUSBITCHARS \"" << def->busBitChars[0] << def->busBitChars[1] << "\" ;" << std::endl;
  f << "DESIGN " << def->designName << " ;" << std::endl;
  f << "UNITS" << def->units << std::endl;
  f << "DIEAREA ";
  for(size_t i = 0; i < def->dieArea.size(); ++i)
    f << "( " << def->dieArea[i].x << " " << def->dieArea[i].y << " ) ";
  f << ";" << std::endl;
  return true;
}

bool DEF_Writer::WriteRows() {
  for (size_t i = 0; i < def->rows.size(); ++i) {
    f << "ROW " << def->rows[i].rowName << " " << def->rows[i].siteName << " " << def->rows[i].origXY.x << " " << def->rows[i].origXY.y;
    switch (def->rows[i].orientation) {
      case Orientation::North         :  f << " N DO "; break;
      case Orientation::South         :  f << " S DO "; break;
      case Orientation::West          :  f << " W DO "; break;
      case Orientation::East          :  f << " E DO "; break;
      case Orientation::FlippedNorth  :  f << " FN DO "; break;
      case Orientation::FlippedSouth  :  f << " FS DO "; break;
      case Orientation::FlippedWest   :  f << " FW DO "; break;
      case Orientation::FlippedEast   :  f << " FE DO "; break;
    }
    f << def->rows[i].numXY.x << " BY " << def->rows[i].numXY.y << " STEP " << def->rows[i].stepXY.x << " " << def->rows[i].stepXY.y << " ;" << std::endl;
  }
  return true;
}

bool DEF_Writer::WriteTracks() {
  for (size_t i = 0; i < def->tracks.size(); ++i) {
    switch (def->tracks[i].direction) {
      case TrackDirection::X : 
        f << "TRACKS X " << def->tracks[i].start << " DO " << def->tracks[i].numTracks << " STEP " << def->tracks[i].step << " LAYER " << def->tracks[i].layerName << " ;" << std::endl;
        break;
      case TrackDirection::Y : 
        f << "TRACKS Y " << def->tracks[i].start << " DO " << def->tracks[i].numTracks << " STEP " << def->tracks[i].step << " LAYER " << def->tracks[i].layerName << " ;" << std::endl;
        break;
    }
  }
  return true;
}

bool DEF_Writer::WriteGCellGrids() {
  //f << "GCELLGRID X " << def->gCellGrid.startX "0 DO 29 STEP 6900 ;" << std::endl;
  for (size_t i = 0; i < def->gCellGrids.size(); ++i)
    f << "GCELLGRID" << def->gCellGrids[i] << std::endl;
  return true;
}

bool DEF_Writer::WriteVIAs() {
  f << "VIAS " << def->vias.size() << " ;" << std::endl;
  for (size_t i = 0; i < def->vias.size(); ++i) {
    VIA &via = def->vias[i];
    f << "    - " << via.name << " + VIARULE " << via.rule << " + CUTSIZE " << via.cutSize.x << " " << via.cutSize.y
      << "  + LAYERS " << via.met1 << " " << via.via << " " << via.met2
      << "  + CUTSPACING " << via.cutSpacing.x << " " << via.cutSpacing.y
      << "  + ENCLOSURE " << via.enclosure.leftBottom.x << " " << via.enclosure.leftBottom.y << " " << via.enclosure.rightTop.x << " " << via.enclosure.rightTop.y;
    if (!via.givenRowCol) {
      f << "  ;" << std::endl;
      continue;
    }
    f << "  + ROWCOL " << via.rowCol.x << " " << via.rowCol.y << "  ;" << std::endl;
  }
  f << "END VIAS" << std::endl;
  return true;
}

bool DEF_Writer::WriteComponents() {
  f << "COMPONENTS " << def->components.size() << " ;" << std::endl;
  for (size_t i = 0; i < def->components.size(); ++i) {
    Component &component = def->components[i];
    f << "    - " << component.name << " " << component.cellName;
    switch (component.source) {
      case CellSource::dist     : f << " + SOURCE DIST"; break;
      case CellSource::netlist  : f << " + SOURCE NETLIST"; break;
      case CellSource::timing   : f << " + SOURCE TIMING"; break;
      case CellSource::user     : f << " + SOURCE USER"; break;
    }
    std::string orientation;
    switch (component.orientation) {
      case Orientation::North       :  orientation = "N";  break;
      case Orientation::South       :  orientation = "S";  break;
      case Orientation::East        :  orientation = "E";  break;
      case Orientation::West        :  orientation = "W";  break;
      case Orientation::FlippedNorth:  orientation = "FN";  break;
      case Orientation::FlippedSouth:  orientation = "FS";  break;
      case Orientation::FlippedEast :  orientation = "FE";  break;
      case Orientation::FlippedWest :  orientation = "FW";  break;
    }

    switch (component.placement) {
      case CellPlacement::placed:
        f << " + PLACED ( " << component.xy.x << " " << component.xy.y << " ) " << orientation << " ;" << std::endl;
        break;
      case CellPlacement::fixed:
        f << " + FIXED ( " << component.xy.x << " " << component.xy.y << " ) " << orientation << " ;" << std::endl;
        break;
      case CellPlacement::cover:
        f << " + COVER ( " << component.xy.x << " " << component.xy.y << " ) " << orientation << " ;" << std::endl;
        break;
      case CellPlacement::unplaced:
        f << " + UNPLACED ;" << std::endl;
        break;
    }
  }
  f << "END COMPONENTS" << std::endl;
  return true;
}

bool DEF_Writer::WritePins() {
  f << "PINS " << def->nPins << " ;";
  for (size_t i = 0; i < def->pins.size(); ++i) {
    f << def->pins[i] << std::endl;
  }
  return true;
}

bool DEF_Writer::WriteSpecialNets() {
  f << "SPECIALNETS " << def->nSpecialNets << " ;";
  for (size_t i = 0; i < def->specialNets.size(); ++i) {
    f << def->specialNets[i] << std::endl;
  }
  return true;
}

bool DEF_Writer::WriteNets() {
  f << "NETS " << def->nets.size() << " ;" << std::endl;
  for (size_t i = 0; i < def->nets.size(); ++i) {
    Net &net = def->nets[i];
    f << "    - " << net.name;
    for (size_t j = 0; j < net.connections.size(); ++j) {
      f << " ( " << net.connections[j].deviceName << " " << net.connections[j].pinName << " )";
      if (net.connections.size() > 7 && j && j % 6 == 0)
        f << std::endl << "     ";
    }
    switch (net.use) {
      case NetUse::signal : f << " + USE SIGNAL" << std::endl; break;
      case NetUse::analog : f << " + USE ANALOG" << std::endl; break;
      case NetUse::clock  : f << " + USE CLOCK" << std::endl; break;
      case NetUse::ground : f << " + USE GROUND" << std::endl; break;
      case NetUse::power  : f << " + USE POWER" << std::endl; break;
      case NetUse::reset  : f << " + USE RESET" << std::endl; break;
      case NetUse::scan   : f << " + USE SCAN" << std::endl; break;
      case NetUse::tieoff : f << " + USE TIEOFF" << std::endl; break;
    }

    RoutedMetal &metal = net.routingPoints[0];
    f << "      + ROUTED " << metal.name << " ( " << metal.first.x << " " << metal.first.y << " )";
    switch (metal.mode) {
      case LastPointMode::point : 
        if (metal.first.x == metal.last.leftBottom.x)
          f << " ( * ";
        else
          f << " ( " << metal.last.leftBottom.x << " ";
        
        if (metal.first.y == metal.last.leftBottom.y)
          f << "* )" << std::endl;
        else
          f << metal.last.leftBottom.y << " )" << std::endl;
        break;
      case LastPointMode::rect  :
        f << " RECT ( " << metal.last.leftBottom.x << " " << metal.last.leftBottom.y << " "
          << metal.last.rightTop.x << " " << metal.last.rightTop.y << " ) " << std::endl;
        break;
      case LastPointMode::via   :
        f << " " << metal.viaName << std::endl;
        break;
    }

    for (size_t j = 1; j < net.routingPoints.size() - 1; ++j) {
      RoutedMetal &metal = net.routingPoints[j];
      
      f << "      NEW " << metal.name << " ( " << metal.first.x << " " << metal.first.y << " )";
      switch (metal.mode) {
      case LastPointMode::point:
        if (metal.first.x == metal.last.leftBottom.x)
          f << " ( * ";
        else
          f << " ( " << metal.last.leftBottom.x << " ";

        if (metal.first.y == metal.last.leftBottom.y)
          f << "* )" << std::endl;
        else
          f << metal.last.leftBottom.y << " )" << std::endl;
        break;
      case LastPointMode::rect:
        f << " RECT ( " << metal.last.leftBottom.x << " " << metal.last.leftBottom.y << " "
          << metal.last.rightTop.x << " " << metal.last.rightTop.y << " ) " << std::endl;
        break;
      case LastPointMode::via:
        f << " " << metal.viaName << std::endl;
        break;
      }
    }

    // TODO: Нужно подумать, как не выносить последюю итерацию
    // Пока что выношу исключительно из-за того, что в конце должна стоять ';'
    metal = net.routingPoints.back();
    f << "      NEW " << metal.name << " ( " << metal.first.x << " " << metal.first.y << " )";
    switch (metal.mode) {
    case LastPointMode::point:
      if (metal.first.x == metal.last.leftBottom.x)
        f << " ( * ";
      else
        f << " ( " << metal.last.leftBottom.x << " ";

      if (metal.first.y == metal.last.leftBottom.y)
        f << "* ) ;" << std::endl;
      else
        f << metal.last.leftBottom.y << " ) ;" << std::endl;
      break;
    case LastPointMode::rect:
      f << " RECT ( " << metal.last.leftBottom.x << " " << metal.last.leftBottom.y << " "
        << metal.last.rightTop.x << " " << metal.last.rightTop.y << " )  ;" << std::endl;
      break;
    case LastPointMode::via:
      f << " " << metal.viaName << " ;" << std::endl;
      break;
    }

  }
  f << "END NETS" << std::endl;
  return true;
}

bool DEF_Writer::WriteFooter() {
  f << "END DESIGN" << std::endl;
  return true;
}
