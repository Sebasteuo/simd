CXX=g++
CXXFLAGS=-O3 -std=c++20 -Wall -Wextra
LDFLAGS=

BIN=bench
SRC=src/utils.cpp src/generator.cpp src/case_converter_serial.cpp src/case_converter_SIMD.cpp src/bench.cpp

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(BIN) src/*.o
