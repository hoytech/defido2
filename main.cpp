#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include <docopt/docopt.h>

#include "util.h"



namespace defido2 {

void cmd_init(const std::vector<std::string> &subArgs);
void cmd_list(const std::vector<std::string> &subArgs);
void cmd_add_priv(const std::vector<std::string> &subArgs);
void cmd_deploy(const std::vector<std::string> &subArgs);
void cmd_sign(const std::vector<std::string> &subArgs);
void cmd_eth(const std::vector<std::string> &subArgs);
void cmd_erc20(const std::vector<std::string> &subArgs);

void cmd_aave(const std::vector<std::string> &subArgs);
void cmd_compound(const std::vector<std::string> &subArgs);
void cmd_uniswap(const std::vector<std::string> &subArgs);



static const char USAGE[] =
R"(
    Usage: defido2 [--config=<config>] [--device=<device>] <command> [<args>...]

    Options:
      --config=<config>     Config file (default defido2.json)
      --device=<device>     FIDO2 device (default /dev/hidraw1)
      -h --help             Show this screen.
      --version             Show version.

    Commands:
      list       Lists available fido2 keys
      init       Inits a key
      add-priv   Adds an ethereum private key for sending transactions
      deploy     Deploy wallet
      sign       Signs a message
      eth        ETH operations
      erc20      ERC20 operations
      compound   Compound operations
      uniswap    Uniswap operations
      aave       AAVE operations
)";




std::string configFile;
std::string fido2Device;


int parse_command_line(int argc, char **argv) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, "defido2 0.0.1", true);

    if (args["--config"]) {
        configFile = args["--config"].asString();
    } else {
        configFile = "defido2.json";
    }

    if (args["--device"]) {
        fido2Device = args["--device"].asString();
    } else if (file_exists(configFile)) {
        auto config = loadConfig();
        if (config.optional<std::string>("device")) fido2Device = config.at("device").get_string();
    }

    if (!fido2Device.size()) fido2Device = "/dev/hidraw1";

    std::string command = args["<command>"].asString();

    if (command == "list") {
        defido2::cmd_list(args["<args>"].asStringList());
    } else if (command == "init") {
        defido2::cmd_init(args["<args>"].asStringList());
    } else if (command == "add-priv") {
        defido2::cmd_add_priv(args["<args>"].asStringList());
    } else if (command == "deploy") {
        defido2::cmd_deploy(args["<args>"].asStringList());
    } else if (command == "sign") {
        defido2::cmd_sign(args["<args>"].asStringList());
    } else if (command == "eth") {
        defido2::cmd_eth(args["<args>"].asStringList());
    } else if (command == "erc20") {
        defido2::cmd_erc20(args["<args>"].asStringList());
    } else if (command == "compound") {
        defido2::cmd_compound(args["<args>"].asStringList());
    } else if (command == "uniswap") {
        defido2::cmd_uniswap(args["<args>"].asStringList());
    } else if (command == "aave") {
        defido2::cmd_aave(args["<args>"].asStringList());
    } else {
        throw hoytech::error("unrecognized command");
    }

    return 0;
}

}


int main(int argc, char **argv) {
    try {
        defido2::parse_command_line(argc, argv);
    } catch (std::exception &e) {
        std::cerr << "CAUGHT EXCEPTION, ABORTING: " << e.what() << std::endl;
        ::exit(1);
    }

    return 0;
}
