#include <stdlib.h>

#include <docopt/docopt.h>

#include "solidityAbi.h"
#include "sign.h"
#include "util.h"


namespace defido2 {



static const char USAGE[] =
R"( 
    Usage:
      aave balance
      aave deposit dai <amount>

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



// https://docs.aave.com/developers/deployed-contracts/deployed-contract-instances

void cmd_aave(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    auto config = loadConfig();

    if (args["balance"].asBool()) {
        auto info = tao::json::from_string(sendTx(std::string("erc20info 0x3a3a65aab0dd2a17e3f1947ba16138cd37d08c04 0x3dfd23A6c5E8BbcFc9581d2E864a68feb6a076d3")));
        std::cout << "aETH Balance: " << info.at("balance").get_string() << std::endl;
        std::cout << "  lpCore approved: " << info.at("allowance").get_string() << std::endl;

        info = tao::json::from_string(sendTx(std::string("erc20info 0xfC1E690f61EFd961294b3e1Ce3313fBD8aa4f85d 0x3dfd23A6c5E8BbcFc9581d2E864a68feb6a076d3")));
        std::cout << "aDAI Balance: " << info.at("balance").get_string() << std::endl;
        std::cout << "  lpCore approved: " << info.at("allowance").get_string() << std::endl;
    } else if (args["deposit"].asBool()) {
        SolidityAbi::Encoder e(SolidityAbi::functionSelector("deposit(address,uint256,uint256)"));

        std::string toHex = "0x398eC7346DcD622eDc5ae82352F02bE94C62d119"; // lending pool
        std::string to = hoytech::from_hex(toHex);

        if (args["dai"].asBool()) {
            auto amountHex = SolidityAbi::parseEther(args["<amount>"].asString());
            std::string valueHex = "0x0";

            e.addUint256("0x6B175474E89094C44Da98b954EedeAC495271d0F"); // dai  
            e.addUint256(amountHex);  
            e.addUint256("0x0");  
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
