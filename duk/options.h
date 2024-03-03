#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>


struct Options final
{
  static constexpr unsigned int defaultDebugPort = 37851;

  std::string input;
  bool debug;
  unsigned int debugPort;
};


[[nodiscard]]
Options parseOptions(int argc, char* argv[]);


#endif // OPTIONS_H
