#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      eth balance
      eth deposit <amount>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_eth(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["balance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("ethBalance")));
        std::cout << "Wallet ETH Balance: " << info.at("walletBalance").get_string() << std::endl;
        std::cout << "Sender ETH Balance: " << info.at("senderBalance").get_string() << std::endl;
    } else if (args["deposit"].asBool()) {
        auto valueStr = args["<amount>"].asString();
        sendTx("ethDeposit " + valueStr);
    }
}


}
