#include <iostream>
#include <string_view>

extern "C" {
#include <fido.h>
}

#include <docopt/docopt.h>

#include "util.h"


namespace f2sc {


static const char USAGE[] =
R"( 
    Usage:
      list

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_list(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    // Adapted from libfido2/tools/token.c

    fido_dev_info_t *devlist;
    size_t ndevs;
    int r;

    if ((devlist = fido_dev_info_new(64)) == nullptr)
        throw hoytech::error("Failed: fido_dev_info_new");
    if ((r = fido_dev_info_manifest(devlist, 64, &ndevs)) != FIDO_OK)
        throw hoytech::error("Failed: fido_dev_info_manifest:", fido_strerr(r));

    for (size_t i = 0; i < ndevs; i++) {
        const fido_dev_info_t *di = fido_dev_info_ptr(devlist, i);
        std::cout
            << fido_dev_info_path(di)
            << " - " << (uint16_t)fido_dev_info_vendor(di) << ":" << (uint16_t)fido_dev_info_product(di)
            << " (" << fido_dev_info_manufacturer_string(di) << "/" << fido_dev_info_product_string(di) << ")"
            << std::endl;
    }

    fido_dev_info_free(&devlist, ndevs);
}


}
