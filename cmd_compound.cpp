#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      compound balance
      compound mint <token> <amount>
      compound enterMarkets <token>
      compound borrow <token> <amount>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



// https://compound.finance/docs#networks

void cmd_compound(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["balance"].asBool()) {
        auto l = [](std::string tokenName) {
            auto info = tao::json::from_string(sendTx(std::string("erc20info ") + lookupAddress(tokenName)));
            std::cout << tokenName << " balance: " << info.at("balance").get_string() << std::endl;
        };

        l("cETH");
        l("cDAI");
    } else if (args["mint"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("mint()"));

        std::string toHex = lookupAddress(args["<token>"].asString());

        auto valueHex = SolidityAbi::parseEther(args["<amount>"].asString());

        std::string payload = e.finish();

        auto sig = sign(SolidityAbi::numberNormalize(toHex, 20) + SolidityAbi::numberNormalize(valueHex, 32) + payload);

        std::string cmd = "invoke ";
        cmd += toHex + " ";
        cmd += valueHex + " ";
        cmd += hoytech::to_hex(payload, true) + " ";
        cmd += hoytech::to_hex(sig.auth, true) + " ";
        cmd += hoytech::to_hex(sig.sig1, true) + " ";
        cmd += hoytech::to_hex(sig.sig2, true) + " ";

        sendTx(cmd);
    } else if (args["enterMarkets"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("enterMarkets(address[])"));

        std::string toHex = "0x3d9819210A31b4961b30EF54bE2aeD79B9c9Cd3B"; // comptroller
        std::string valueHex = "0x0";

        auto p = e.getArrayPointer();

        std::vector<std::string> v;
        v.emplace_back(SolidityAbi::numberNormalize(lookupAddress(args["<token>"].asString()), 32));
        e.addArrayContents(p, v);

        std::string payload = e.finish();

        auto sig = sign(SolidityAbi::numberNormalize(toHex, 20) + SolidityAbi::numberNormalize(valueHex, 32) + payload);

        std::string cmd = "invoke ";
        cmd += toHex + " ";
        cmd += valueHex + " ";
        cmd += hoytech::to_hex(payload, true) + " ";
        cmd += hoytech::to_hex(sig.auth, true) + " ";
        cmd += hoytech::to_hex(sig.sig1, true) + " ";
        cmd += hoytech::to_hex(sig.sig2, true) + " ";

        sendTx(cmd);
    } else if (args["borrow"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("borrow(uint256)"));

        std::string toHex = lookupAddress(args["<token>"].asString());

        std::string valueHex = "0x0";

        e.addUint256(SolidityAbi::parseEther(args["<amount>"].asString()));
        std::string payload = e.finish();

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
