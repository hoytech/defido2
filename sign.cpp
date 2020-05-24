#include <iostream>
#include <string_view>

#include <unistd.h> // getpass(), FIXME
#include <sys/random.h>

extern "C" {
#include <fido.h>
#include <cbor.h>
}

#include <picosha2.h>

#include "util.h"
#include "sign.h"


namespace defido2 {



SignatureResult sign(std::string_view message) {
    auto config = loadConfig();

    std::string credId = hoytech::from_hex(config.at("credId").get_string());
    std::string clientDataHash = sha256(message);


    fido_assert_t *assert = nullptr;
    fido_dev_t *dev = nullptr;
    int r = 0;

    assert = fido_assert_new();
    if (!assert) throw hoytech::error("Failed: fido_assert_new:", fido_strerr(r));


    if ((r = fido_assert_set_clientdata_hash(assert, reinterpret_cast<unsigned char*>(clientDataHash.data()), clientDataHash.size())) != FIDO_OK) throw hoytech::error("Failed: fido_assert_set_clientdata_hash:", fido_strerr(r));
    if ((r = fido_assert_set_rp(assert, "defido2 verification")) != FIDO_OK) throw hoytech::error("Failed: fido_assert_set_rp:", fido_strerr(r));
    if ((r = fido_assert_allow_cred(assert, reinterpret_cast<const unsigned char*>(credId.data()), credId.size())) != FIDO_OK) throw hoytech::error("Failed: fido_assert_set_rp:", fido_strerr(r));



    if ((dev = fido_dev_new()) == nullptr) throw hoytech::error("Failed: fido_dev_new:", fido_strerr(r));
    if ((r = fido_dev_open(dev, fido2Device.c_str())) != FIDO_OK) throw hoytech::error("Failed: fido_dev_open(", fido2Device, "):", fido_strerr(r));


    std::cout << "Enter PIN for " << fido2Device << ": " << std::flush;

    char *pin = getpass("");
    r = fido_dev_get_assert(dev, assert, pin);
    // FIXME: explicit_bzero(pin)

    if (r != FIDO_OK) throw hoytech::error("Failed: fido_dev_make_cred:", fido_strerr(r));


    for (size_t idx = 0; idx < fido_assert_count(assert); idx++) {

        const unsigned char *authdata_cbor_ptr = fido_assert_authdata_ptr(assert, idx);
        size_t authdata_cbor_len = fido_assert_authdata_len(assert, idx);

        cbor_item_t *item = NULL;
        unsigned char *authdata_ptr = NULL;
        size_t authdata_len;
        struct cbor_load_result cbor;

        cbor_load(authdata_cbor_ptr, authdata_cbor_len, &cbor);

        item = cbor_load(authdata_cbor_ptr, authdata_cbor_len, &cbor);

        authdata_ptr = cbor_bytestring_handle(item);
        authdata_len = cbor_bytestring_length(item);

        //std::cout << "auth: " << hoytech::to_hex(std::string_view(reinterpret_cast<const char *>(authdata_ptr), authdata_len), true) << std::endl;


        std::string sig(reinterpret_cast<const char *>(fido_assert_sig_ptr(assert, idx)), fido_assert_sig_len(assert, idx));

        size_t xlength = sig[3];
        sig = sig.substr(4);

        //std::cout << "sig:  " << hoytech::to_hex(sig.substr(0, xlength), true) << "," << hoytech::to_hex(sig.substr(xlength + 2), true) << std::endl;

        return SignatureResult{
            std::string(reinterpret_cast<const char *>(authdata_ptr), authdata_len),
            sig.substr(0, xlength),
            sig.substr(xlength + 2)
        };
    }

    throw hoytech::error("couldn't find signature");
}


}
