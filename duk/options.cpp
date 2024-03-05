#include "options.h"
#include <boost/program_options.hpp>
#include <sstream>


Options parseOptions(int argc, char* argv[])
{
  namespace po = boost::program_options;

  po::options_description optionsDesc("Options");
  optionsDesc.add_options()
    ("help,h", "show help")
    ("input,i", po::value<decltype(Options::input)>()->required(), "input file")
    ("debug,d", po::value<decltype(Options::debug)>()->default_value(false)->implicit_value(true), "enable debugger")
    ("port,p", po::value<decltype(Options::debugPort)>()->default_value(Options::defaultDebugPort), "debugger port");

  po::variables_map optionsMap;
  po::store(po::parse_command_line(argc, argv, optionsDesc), optionsMap);

  if (optionsMap.count("help"))
  {
    std::ostringstream ss;
    ss << optionsDesc;
    throw po::error(ss.str());
  }

  po::notify(optionsMap);

  return Options {
    .input = optionsMap.at("input").as<decltype(Options::input)>(),
    .debug = optionsMap.at("debug").as<decltype(Options::debug)>(),
    .debugPort = optionsMap.at("port").as<decltype(Options::debugPort)>()
  };
}
