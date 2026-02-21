CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
LDFLAGS = -lws2_32

SRC_DIR = src
BUILD_DIR = build

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Antenna/Antenna.cpp $(SRC_DIR)/Network/AntennaServer.cpp

TARGET = $(BUILD_DIR)/AstroControlSim

all: $(TARGET)

$(TARGET): $(SRCS)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rmdir /s /q $(BUILD_DIR)
