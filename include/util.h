#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

#include <tao/json.hpp>

#include <hoytech/error.h>
#include <hoytech/hex.h>





namespace defido2 {

extern std::string configFile;
extern std::string fido2Device;



static inline std::string sendTx(std::string cmd) {
    auto commandLine = "node sendTx.js '" + configFile + "' " + cmd;

    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(commandLine.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }

    if (pclose(pipe)) throw hoytech::error("error sending tx");

    return result.substr(0, result.size() - 1); // chomp trailing newline
}




static inline tao::json::value loadConfig() {
    return tao::json::from_file(configFile);
}

static inline void saveConfig(tao::json::value &config) {
    std::ofstream out(configFile);
    out << tao::json::to_string(config, 4) << "\n";
    out.close();
}

}
