#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <gmpxx.h>

#include "keccak.h"
#include "util.h"



namespace SolidityAbi {

static inline std::string functionSelector(std::string_view functionSignature) {
    return keccak256(functionSignature).substr(0, 4);
}

static inline std::string numberNormalize(std::string_view hexStr, size_t numBytes) {
    std::string str(hexStr);
    if (str.substr(0, 2) == "0x") str = str.substr(2);
    str.replace(0, 0, std::string(numBytes*2 - str.length(), '0'));
    return hoytech::from_hex(str);
}

static inline std::string numberNormalize(mpz_class &num, size_t numBytes) {
    auto str = num.get_str(16);
    if (str.length() > numBytes*2) throw hoytech::error("input exceeds numBytes");
    return numberNormalize(str, numBytes);
}

static inline std::string numberNormalize(uint64_t num, size_t numBytes) {
    mpz_class numMpz{num};
    return numberNormalize(numMpz, numBytes);
}

// FIXME: move to ::util
static inline mpz_class convertToMpz(std::string_view str) {
    if (str.size() == 0) return mpz_class(0);
    std::string s = hoytech::to_hex(str);
    return mpz_class{s, 16};
}

// FIXME: move to ::util
static inline mpz_class convertToMpzTwosComplement(std::string_view str) {
    // FIXME: Find better way to do this
    static const mpz_class twoToThe256("115792089237316195423570985008687907853269984665640564039457584007913129639936");

    auto num = convertToMpz(str);

    if (mpz_tstbit(num.get_mpz_t(), 255)) {
        return -(twoToThe256 - num);
    }

    return num;
}




class Encoder {
  public:
    Encoder(std::string_view selector) {
        if (selector.length() != 4) throw hoytech::error("function selector must be 4 bytes");
        buffer += selector;
    }

    void addUint256(std::string_view hexStr) {
        if (hexStr.substr(0, 2) == "0x") hexStr = hexStr.substr(2);
        addRaw(numberNormalize(hexStr, 32));
    }

    void addUint256(uint64_t n) {
        mpz_class num{n};
        addRaw(numberNormalize(num, 32));
    }

    void addRaw(std::string_view u) {
        buffer += u;
    }

    size_t getArrayPointer() {
        size_t offset = buffer.size();
        addRaw(std::string(32, '\0'));
        return offset;
    }
    
    void addArrayContents(size_t arrayPointer, std::vector<std::string_view> &a) {
        mpz_class currOffset{buffer.size() - 4};
        buffer.replace(arrayPointer, 32, numberNormalize(currOffset, 32));

        mpz_class arrayElems{a.size()};
        addRaw(numberNormalize(arrayElems, 32));
        if (a.size() == 0) return;
        size_t elemSize = a[0].size();

        for (auto &e : a) {
            if (e.size() != elemSize) throw hoytech::error("array elements must all be the same size");
            buffer += e;
        }
    }

    void addArrayContents(size_t arrayPointer, std::vector<std::string> &a) {
        std::vector<std::string_view> svs;
        for (auto &e : a) svs.push_back(e);
        addArrayContents(arrayPointer, svs);
    }

    std::string finish() {
        return std::move(buffer);
    }

  private:
    std::string buffer;
};



class Decoder {
  public:
    Decoder(std::string_view buffer_) : buffer(buffer_) {}

    std::string_view get32Bytes() {
        auto output = buffer.substr(currOffset, 32);
        if (output.size() != 32) throw hoytech::error("tried to access 32 bytes, but not enough in buffer");
        currOffset += 32;
        return output;
    }

    uint64_t getUint64() {
        return convertToMpz(get32Bytes()).get_ui();
    }

    std::vector<std::string_view> getArray(size_t elemSize) {
        size_t arrayOffset = convertToMpz(get32Bytes()).get_ui();
        size_t numElems = convertToMpz(buffer.substr(arrayOffset, 32)).get_ui();

        std::vector<std::string_view> output;

        for (size_t i = 0; i < numElems; i++) {
            output.push_back(buffer.substr(arrayOffset + 32 + (i * elemSize), elemSize));
        }

        return output;
    }

  private:
    std::string_view buffer;
    size_t currOffset = 0;
};



}
