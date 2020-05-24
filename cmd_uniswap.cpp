#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      uniswap getpair <token1> <token2>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



// https://uniswap.org/docs/v2

void cmd_uniswap(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["getpair"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("uniswapGetPair ") + args["<token1>"].asString() + " " + args["<token2>"].asString()));
        std::cout << "Pair address: " << info.at("pair").get_string() << std::endl;
    }
}


}
