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
        auto token = lookupAddress(args["<token>"].asString());
        auto info = tao::json::from_string(sendTx(std::string("erc20info ") + token));
        std::cout << "Balance: " << info.at("balance").get_string() << std::endl;
    } else if (args["allowance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("erc20info ") + args["<token>"].asString() + " " + args["<spender>"].asString()));
        std::cout << "Allowance: " << info.at("allowance").get_string() << std::endl;
    } else if (args["approve"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("approve(address,uint256)"));
        e.addUint256(args["<spender>"].asString());
        e.addUint256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        std::string payload = e.finish();

        std::string toHex = args["<token>"].asString();
        std::string valueHex = "0x0000000000000000000000000000000000000000000000000000000000000000";

        auto sig = sign(SolidityAbi::numberNormalize(toHex, 20) + SolidityAbi::numberNormalize(valueHex, 32) + payload);

        std::string cmd = "invoke ";
        cmd += toHex + " ";
        cmd += valueHex + " ";
        cmd += hoytech::to_hex(payload, true) + " ";
        cmd += hoytech::to_hex(sig.auth, true) + " ";
        cmd += hoytech::to_hex(sig.sig1, true) + " ";
        cmd += hoytech::to_hex(sig.sig2, true) + " ";

        sendTx(cmd);
    }
}


}
