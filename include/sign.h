#pragma once

#include "util.h"


namespace defido2 {

struct SignatureResult {
    std::string auth;
    std::string sig1;
    std::string sig2;
};

SignatureResult sign(std::string_view message);

}
