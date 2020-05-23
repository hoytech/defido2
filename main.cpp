#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include <docopt/docopt.h>

#include "util.h"



namespace defido2 {

void cmd_init(const std::vector<std::string> &subArgs);
void cmd_list(const std::vector<std::string> &subArgs);
void cmd_sign(const std::vector<std::string> &subArgs);

}


static const char USAGE[] =
R"(
    Usage: defido2 <command> [<args>...]

    Options:
      -h --help             Show this screen.
      --version             Show version.

    Commands:
      list       Lists available keys
      init       Inits a key
      sign       Signs a message
)";





int parse_command_line(int argc, char **argv) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "defido2 0.0.1", true);

    std::string command = args["<command>"].asString();

    if (command == "list") {
        defido2::cmd_list(args["<args>"].asStringList());
    } else if (command == "init") {
        defido2::cmd_init(args["<args>"].asStringList());
    } else if (command == "sign") {
        defido2::cmd_sign(args["<args>"].asStringList());
    } else {
        throw hoytech::error("unrecognized command");
    }

    return 0;
}


int main(int argc, char **argv) {
    try {
        parse_command_line(argc, argv);
    } catch (std::exception &e) {
        std::cerr << "CAUGHT EXCEPTION, ABORTING: " << e.what() << std::endl;
        ::exit(1);
    }

    return 0;
}
