CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -lpqxx -lpq -lpthread -lprotobuf

# Use pkg-config for including protobuf flags
PROTOBUF_FLAGS = $(shell pkg-config --cflags --libs protobuf)

SOURCES = main.cpp server.cpp message.cpp handleworld.cpp handleinit.cpp handleamazon.cpp database.cpp amazon_ups.pb.cc world_ups.pb.cc
OBJECTS = $(SOURCES:.cpp=.o)

# Protobuf generation rule
%.pb.cc %.pb.h: %.proto
	protoc --cpp_out=. $<

# 可执行文件名
EXECUTABLE = server_app

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) amazon_ups.pb.cc world_ups.pb.cc
	$(CXX) $(CXXFLAGS) $(PROTOBUF_FLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PROTOBUF_FLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY: all clean
