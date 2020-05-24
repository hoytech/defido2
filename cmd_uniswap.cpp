#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      uniswap swap <amountIn> <tokenIn> <tokenOut>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



// https://uniswap.org/docs/v2

void cmd_uniswap(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto amountInHex = SolidityAbi::parseEther(args["<amountIn>"].asString());
    std::string tokenInHex = lookupAddress(args["<tokenIn>"].asString());
    std::string tokenOutHex = lookupAddress(args["<tokenOut>"].asString());

    auto config = loadConfig();

    if (args["swap"].asBool()) {
        std::string toHex = lookupAddress("uniswap");

        SolidityAbi::Encoder e(SolidityAbi::functionSelector("swapExactTokensForTokens(uint256,uint256,address[],address,uint256)"));

        e.addUint256(amountInHex);
        e.addUint256("0x0");
        auto p = e.getArrayPointer();
        e.addUint256(config["walletAddr"].get_string());
        e.addUint256(hoytech::to_hex(SolidityAbi::numberNormalize(::time(nullptr) + 86400, 32), true));

        std::vector<std::string> v;
        v.emplace_back(SolidityAbi::numberNormalize(tokenInHex, 32));
        v.emplace_back(SolidityAbi::numberNormalize(tokenOutHex, 32));
        e.addArrayContents(p, v);

        std::string payload = e.finish();
        std::string valueHex = "0x0";

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
