#include <iostream>
#include <string_view>

#include <unistd.h> // getpass(), FIXME
#include <sys/random.h>

extern "C" {
#include <fido.h>
}

#include <docopt/docopt.h>

#include <picosha2.h>

#include "util.h"

//FIXME kill
extern "C" {
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <fido/es256.h>
}

namespace f2sc {


static const char USAGE[] =
R"( 
    Usage:
      init [--device=<device>]

    Options:
      --device=<device>     FIDO2 device.
      -h --help             Show this screen.
      --version             Show version.
)";



void cmd_init(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    std::string device = "/dev/hidraw2";
    if (args["--device"]) device = args["--device"].asString();

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
    if ((r = fido_dev_open(dev, device.c_str())) != FIDO_OK) throw hoytech::error("Failed: fido_dev_open(", device, "):", fido_strerr(r));


    r = fido_dev_make_cred(dev, cred, nullptr);
    if (r == FIDO_ERR_PIN_REQUIRED) {
        std::cout << "Enter PIN for " << device << ": " << std::flush;

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


    std::cout << "cred id: " << hoytech::to_hex(std::string_view(reinterpret_cast<const char *>(fido_cred_id_ptr(cred)), fido_cred_id_len(cred)), true) << std::endl;
    std::cout << "pub key: " << hoytech::to_hex(std::string_view(reinterpret_cast<const char *>(fido_cred_pubkey_ptr(cred)), fido_cred_pubkey_len(cred)), true) << std::endl;

    auto *ptr = fido_cred_pubkey_ptr(cred);
    auto len = fido_cred_pubkey_len(cred);

//FIXME kill
        EVP_PKEY *pkey = nullptr;
        es256_pk_t *pk = nullptr;
        int ok = -1;

        if ((pk = es256_pk_new()) == NULL) {
        }

        if (es256_pk_from_ptr(pk, ptr, len) != FIDO_OK) {
        }

        if ((pkey = es256_pk_to_EVP_PKEY(pk)) == NULL) {
        }

        if (PEM_write_PUBKEY(::stdout, pkey) == 0) {
        }
}


}
