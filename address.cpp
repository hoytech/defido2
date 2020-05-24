#include <string>
#include <algorithm>

#include "util.h"

namespace defido2 {

std::string lookupAddress(std::string spec) {
    if (spec.substr(0,2) == "0x") return spec;

    std::transform(spec.begin(), spec.end(), spec.begin(), ::tolower);

    if (spec == "dai") return "0x6B175474E89094C44Da98b954EedeAC495271d0F";
    if (spec == "weth") return "0xC02aaA39b223FE8D0A0e5C4F27eAD9083C756Cc2";
    if (spec == "cdai") return "0x5d3a536E4D6DbD6114cc1Ead35777bAB948E3643";
    if (spec == "ceth") return "0x4Ddc2D193948926D02f9B1fE9e1daa0718270ED5";

    throw hoytech::error("unable to lookup address: ", spec);
}

}
