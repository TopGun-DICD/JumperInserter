// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <iostream>

#include "DEF.hpp"
#include "DEF_Reader.hpp"
#include "AntennaViolation.hpp"
#include "AntennaReport_Reader.hpp"
#include "JumperInsterter.hpp"
#include "DEF_Writer.hpp"

// --def tests/lut_s44.def --report tests/39-antenna-1.log
// --def tests/lut_s44.def --report tests/39-antenna-1.log > report.log

enum class ProcessMode : uint8_t {
  shortMode,                        // __________ -> ___|-|___
  longMode,                         // __________ -> ____|----|
};

struct InputData {
  std::string fileNameDEF,
              fileNameAReport,
              fileNameLEF;
  ProcessMode mode = ProcessMode::shortMode;
};

bool ParseArgs(int argc, char *argv[], InputData &args);

int main(int argc, char* argv[]) {
  std::cout << "JumperInserter v0.0.3" << std::endl << std::endl;
  InputData args;
  if (!ParseArgs(argc, argv, args))
    return EXIT_FAILURE;

  DEF                   def;
  DEF_Reader            defReader;
  if (!defReader.Read(args.fileNameDEF, &def))
    return EXIT_FAILURE;
  defReader.PrintStatistics();

  AntennaViolations     violations;
  AntennaReport_Reader  antennaReader;
  if (!antennaReader.Read(args.fileNameAReport, &violations))
    return EXIT_FAILURE;
  antennaReader.PrintStatistics();

  JumperInserter jumperInserter;
  if (!jumperInserter.ProcessViolatedNets(def, violations, true))
    return EXIT_FAILURE;
  jumperInserter.PrintStatistics();

  DEF_Writer              writer;
  writer.Write("result.def", &def);
  writer.PrintStatistics();
  
  return EXIT_SUCCESS;
}

bool ParseArgs(int argc, char* argv[], InputData& args) {
  if (argc == 2) {
    if (!strcmp(argv[1], "--help")) {
      std::cout << "JumperInserter is a software to aviod antenna violations in VLSI layouts." << std::endl;
      std::cout << "Run program with the next arguments:" << std::endl << std::endl;
      std::cout << "\tJumperInserter --help" << std::endl;
      std::cout << "\t           (Will show this screen)" << std::endl;
      std::cout << "\tJumperInserter --def <DEF> --report <REPORT> --lef <LEF> [--mode <MODE>]" << std::endl;
      std::cout << "Here:" << std::endl;
      std::cout << "\t<DEF>    - DEF file name" << std::endl;
      std::cout << "\t<REPORT> - Antenna report file" << std::endl;
      std::cout << "\t<LEF>    - Tech LEF" << std::endl;
      std::cout << "\t<MODE>   - Jumper insertion mode, one of the following:" << std::endl;
      std::cout << "\t\tlong : __________ -> _____|-----|" << std::endl;
      std::cout << "\t\tshort: __________ -> _____|-|____ (by default)" << std::endl;
    }
    else {
      std::cerr << "__err__ The only allowed single agument is '--help'." << std::endl;
      std::cerr << "        Run 'JumperInserter --help' for details." << std::endl << "Abort." << std::endl;
      return false;
    }
    return false;
  }

  if (argc < 5) {
    std::cerr << "Not enough arguments!" << std::endl << "Should be:" << std::endl;
    std::cerr << "\tJumperInserter --def <DEF> --report <REPORT> --lef <LEF> [--mode <MODE>]" << std::endl << std::endl;
    std::cerr << "Run JumperInserter --help for details." << std::endl;
    return false;
  }

  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--help")) {
      std::cout << "Run program as following:" << std::endl << std::endl;
      std::cout << "\tJumperInserter --def <DEF> --report <REPORT> --lef <LEF> [--mode <MODE>]" << std::endl << std::endl;
      std::cout << "<DEF>    - DEF file name" << std::endl;
      std::cout << "<REPORT> - Antenna report file" << std::endl;
      std::cout << "<LEF>    - Tech LEF" << std::endl;
      std::cout << "<MODE>   - Jumper insertion mode, one of the following:" << std::endl;
    }
    if (!strcmp(argv[i], "--def")) {
      if (i < argc - 1) {
        args.fileNameDEF = argv[++i];
        continue;
      }
      else {
        std::cerr << "__err__ Command line argument '--def' should be followed by DEF file name." << std::endl;
        std::cerr << "        Run 'JumperInserter --help' for details." << std::endl << "Abort." << std::endl;
        return false;
      }
    }
    if (!strcmp(argv[i], "--report")) {
      if (i < argc - 1) {
        args.fileNameAReport = argv[++i];
        continue;
      }
      else {
        std::cerr << "__err__ Command line argument '--report' should be followed by antenna report file name." << std::endl;
        std::cerr << "        Run 'JumperInserter --help' for details." << std::endl << "Abort." << std::endl;
        return false;
      }
    }
  }
  return true;
}

