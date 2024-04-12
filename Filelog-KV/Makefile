# Compiler and flags
CXX := g++
CFLAGS := -std=c++17
LFLAGS := -lrpc -lpthread

# Directories
RPCLIB_DIR := /usr/local/include/
RPDJSON_DIR := rapidjson/
INC_DIR := -I$(RPCLIB_DIR) -I$(RPDJSON_DIR)

# Targets
TARGETS := client gateway logger 

# Source files
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)  # List of object files

# Rule to build all targets
all: $(TARGETS)

# Rule to build debug targets
debug: DFLAGS := -ggdb -DDEBUG
debug: $(TARGETS)

# Rule to compile object files
%.o: %.cpp
	$(CXX) -c $< -o $@ $(INC_DIR) $(CFLAGS) $(DFLAGS)

# Rule to link each target
$(TARGETS): % : %.o read_config.o
	$(CXX) $^ -o $@ $(LFLAGS) -L$(LIB_DIR) -lrpc

# Rule to clean compiled files
clean:
	rm -f $(OBJS) $(TARGETS)

.PHONY: all debug clean