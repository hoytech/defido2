#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      erc20 balance <token>
      erc20 allowance <token> <spender>
      erc20 approve <token> <spender>
      erc20 send <token> <amount> <to>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_erc20(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["balance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("erc20info ") + args["<token>"].asString()));
        std::cout << "Balance: " << info.at("balance").get_string() << std::endl;
    } else if (args["allowance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("erc20info ") + args["<token>"].asString() + " " + args["<spender>"].asString()));
        std::cout << "Balance: " << info.at("balance").get_string() << std::endl;
    } else if (args["approve"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("approve(address,uint256)"));
        e.addUint256(args["<spender>"].asString());
        e.addUint256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        std::string abiMsg = e.finish();

        std::string token = hoytech::from_hex(args["<token>"].asString());

        auto payload = token + abiMsg;
        auto sig = sign(payload);

        std::string cmd = "doSend ";
        cmd += args["<token>"].asString() + " ";
        cmd += hoytech::to_hex(payload, true) + " ";
        cmd += hoytech::to_hex(sig.auth, true) + " ";
        cmd += hoytech::to_hex(sig.sig1, true) + " ";
        cmd += hoytech::to_hex(sig.sig2, true) + " ";

        std::cout << sendTx(cmd) << std::endl;
    }
}


}
