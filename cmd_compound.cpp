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
      compound mint cETH <amount>
      compound enterMarkets cETH
      compound borrow DAI <amount>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



// https://compound.finance/docs#networks

void cmd_compound(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["balance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("erc20info 0x4Ddc2D193948926D02f9B1fE9e1daa0718270ED5")));
        std::cout << "cETH Balance: " << info.at("balance").get_string() << std::endl;

        info = tao::json::from_string(sendTx(std::string("erc20info 0x5d3a536E4D6DbD6114cc1Ead35777bAB948E3643")));
        std::cout << "cDAI Balance: " << info.at("balance").get_string() << std::endl;
    } else if (args["mint"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("mint()"));

        std::string toHex = "0x4Ddc2D193948926D02f9B1fE9e1daa0718270ED5"; // cETH

        if (args["cETH"].asBool()) {
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
        }
    } else if (args["enterMarkets"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("enterMarkets(address[])"));

        std::string toHex = "0x3d9819210A31b4961b30EF54bE2aeD79B9c9Cd3B"; // comptroller
        std::string valueHex = "0x0";

        auto p = e.getArrayPointer();

        std::vector<std::string> v;
        if (args["cETH"].asBool()) v.emplace_back(SolidityAbi::numberNormalize("0x4Ddc2D193948926D02f9B1fE9e1daa0718270ED5", 32));
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

        std::string toHex = "0x5d3a536E4D6DbD6114cc1Ead35777bAB948E3643"; // cDAI

        if (args["DAI"].asBool()) {
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


}
