# Compiler and flags
CXX := g++
CFLAGS := -std=c++14
LFLAGS := -lrpc -lpthread

# Directories
RPCLIB_DIR := /usr/local/include/
INC_DIR := -I$(RPCLIB_DIR)

# Targets
TARGETS := client gateway logger

# Source files
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=)

# Rule to build all targets
all: $(TARGETS)

# Rule to build debug targets
debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGETS)

# Rule to compile object files
%: %.cpp
	$(CXX)  $< -o $@ $(INC_DIR) $(CFLAGS) $(DFLAGS) $(LFLAGS)

# Rule to clean compiled files
clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all debug clean
