#include <iostream>
#include <string_view>

#include <docopt/docopt.h>

namespace f2sc {


static const char USAGE[] =
R"( 
    Usage:
      init

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_init(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    std::cout << "Doing init" << std::endl;
}


}
