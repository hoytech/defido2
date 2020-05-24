#include <stdlib.h>

#include <docopt/docopt.h>

#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      deploy

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_deploy(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    auto privKey = config.optional<std::string>("privKey");
    if (!privKey) throw hoytech::error("Must add private key before deploying");

    if (config.optional<std::string>("walletAddr")) throw hoytech::error("Wallet already deployed");

    auto walletAddr = sendTx("deploy " + config.at("pubKey").get_string());
    std::cout << "Wallet deployed to " << walletAddr << std::endl;

    config["walletAddr"] = walletAddr;

    saveConfig(config);
}


}
