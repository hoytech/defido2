W        = -Wall
OPT      = -O2 -g
STD      = -std=c++17
CXXFLAGS = $(STD) $(OPT) $(W) -fPIC $(XCXXFLAGS)
INCS     = -Iinclude -Iexternal -Iexternal/hoytech-cpp -Iexternal/json/include

LDLIBS   = -lfido2 -lcbor -lssl -lcrypto -ldocopt -lgmp -lgmpxx
LDFLAGS  = -flto $(XLDFLAGS)

SRCS     = main.cpp sign.cpp keccak.cpp address.cpp external/hoytech-cpp/hex.cpp
CMDSRCS  = cmd_list.cpp cmd_init.cpp cmd_add_priv.cpp cmd_deploy.cpp cmd_sign.cpp cmd_eth.cpp cmd_erc20.cpp cmd_compound.cpp cmd_uniswap.cpp cmd_aave.cpp

BIN      = defido2



OBJS    := $(SRCS:.cpp=.o)
CMDOBJS := $(CMDSRCS:.cpp=.o)
DEPS    := $(SRCS:.cpp=.d) $(CMDSRCS:.cpp=.d)

$(BIN): $(OBJS) $(CMDOBJS) $(DEPS)
	$(CXX) $(OBJS) $(CMDOBJS) $(LDFLAGS) $(LDLIBS) -o $(BIN)

%.o : %.cpp %.d
	$(CXX) $(CXXFLAGS) $(INCS) -MMD -MP -MT $@ -MF $*.d -c $< -o $@

-include *.d

%.d : ;

.PHONY: clean asan
clean:
	rm -f $(BIN) *.o *.d
	rm -rf build/ package/

asan: XCXXFLAGS = -fsanitize=address
asan: XLDFLAGS = -fsanitize=address
asan: $(BIN)



SOLC ?= solc

build/Defido2.json: contracts/*.sol
	mkdir -p build/
	$(SOLC) --optimize --combined-json abi,bin contracts/Defido2.sol > build/Defido2.json.tmp
	mv build/Defido2.json.tmp build/Defido2.json
