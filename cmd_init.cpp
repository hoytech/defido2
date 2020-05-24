#include <iostream>
#include <string_view>

#include <unistd.h>
#include <sys/random.h>

extern "C" {
#include <fido.h>
}

#include <docopt/docopt.h>

#include <picosha2.h>

#include "util.h"


namespace defido2 {


static bool file_exists(std::string &name) {
    return access(name.c_str(), F_OK) != -1;
}


static const char USAGE[] =
R"( 
    Usage:
      init

    Options:
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_init(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");


    if (file_exists(configFile)) throw hoytech::error("Config file ", configFile, " already exists, refusing to init");
    tao::json::value config = tao::json::empty_object;


    fido_cred_t *cred = nullptr;
    fido_dev_t *dev = nullptr;
    int r = 0;

    cred = fido_cred_new();
    if (!cred) throw hoytech::error("Failed: fido_cred_new:", fido_strerr(r));

    if ((r = fido_cred_set_type(cred, COSE_ES256)) != FIDO_OK) throw hoytech::error("Failed: fido_cred_set_type:", fido_strerr(r));

    std::string clientDataHash = sha256("credential challenge");
    unsigned char uid[32];
    if (getrandom(uid, sizeof(uid), 0) != sizeof(uid)) throw hoytech::error("getrandom failure");

    if ((r = fido_cred_set_clientdata_hash(cred, reinterpret_cast<unsigned char*>(clientDataHash.data()), clientDataHash.size())) != FIDO_OK) throw hoytech::error("Failed: fido_cred_set_clientdata_hash:", fido_strerr(r));
    if ((r = fido_cred_set_rp(cred, "defido2 verification", nullptr)) != FIDO_OK) throw hoytech::error("Failed: fido_cred_set_rp:", fido_strerr(r));
    if ((r = fido_cred_set_user(cred, uid, sizeof(uid), "defido2", nullptr, nullptr)) != FIDO_OK) throw hoytech::error("Failed: fido_cred_set_user:", fido_strerr(r));


    if ((dev = fido_dev_new()) == nullptr) throw hoytech::error("Failed: fido_dev_new:", fido_strerr(r));
    if ((r = fido_dev_open(dev, fido2Device.c_str())) != FIDO_OK) throw hoytech::error("Failed: fido_dev_open(", fido2Device, "):", fido_strerr(r));


    r = fido_dev_make_cred(dev, cred, nullptr);
    if (r == FIDO_ERR_PIN_REQUIRED) {
        std::cout << "Enter PIN for " << fido2Device << ": " << std::flush;

        char *pin = getpass("");
        r = fido_dev_make_cred(dev, cred, pin);
        // FIXME: explicit_bzero(pin)
    }

    if (r != FIDO_OK) throw hoytech::error("Failed: fido_dev_make_cred:", fido_strerr(r));


    if (fido_cred_x5c_ptr(cred) == nullptr) {
        if ((r = fido_cred_verify_self(cred)) != FIDO_OK) throw hoytech::error("Failed: fido_cred_verify_self:", fido_strerr(r));
    } else {
        if ((r = fido_cred_verify(cred)) != FIDO_OK) throw hoytech::error("Failed: fido_cred_verify:", fido_strerr(r));
    }


    config["credId"] = hoytech::to_hex(std::string_view(reinterpret_cast<const char *>(fido_cred_id_ptr(cred)), fido_cred_id_len(cred)), true);
    config["pubKey"] = hoytech::to_hex(std::string_view(reinterpret_cast<const char *>(fido_cred_pubkey_ptr(cred)), fido_cred_pubkey_len(cred)), true);

    saveConfig(config);
    std::cout << "Initialized file " << configFile << std::endl;
}


}
