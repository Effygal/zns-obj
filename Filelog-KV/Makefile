# Compiler and flags
CC := gcc
CXX := g++
CFLAGS := -Wall -std=c++11
LFLAGS := -pthread

# Directories
RPCLIB_DIR := /usr/local/include
INC_DIR := -I$(RPCLIB_DIR)

# Targets
TARGETS := gateway logger client

# Source files
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

# Rule to build all targets
all: $(TARGETS)

# Rule to build debug targets
debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGETS)

# Rule to compile object files
$(OBJS): $(SRCS)
	$(CXX) $(CFLAGS) $(DFLAGS) $(INC_DIR) -c $<

# Rule to build gateway
gateway: gateway.o 
	$(CXX) $(LFLAGS) -o $@ $^

# Rule to build logger
logger: logger.o
	$(CXX) $(LFLAGS) -o $@ $^

# Rule to build client
client: client.o
	$(CXX) $(LFLAGS) -o $@ $^

# Rule to clean compiled files
clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all debug clean
