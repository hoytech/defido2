W        = -Wall
OPT      = -O2 -g
STD      = -std=c++17
CXXFLAGS = $(STD) $(OPT) $(W) -fPIC $(XCXXFLAGS)
INCS     = -Iinclude -Iexternal -Iexternal/hoytech-cpp -Iexternal/json/include

LDLIBS   = -lfido2 -lssl -lcrypto -ldocopt
LDFLAGS  = -flto $(XLDFLAGS)

SRCS     = main.cpp external/hoytech-cpp/hex.cpp
CMDSRCS  = cmd_list.cpp cmd_init.cpp cmd_sign.cpp

BIN      = f2sc



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
