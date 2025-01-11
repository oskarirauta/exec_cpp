all: world

CXX?=g++
CXXFLAGS?=--std=c++17 -Wall -fPIC -g

example_OBJS:= \
	objs/main.o

EXEC_DIR:=.
include ./Makefile.inc

world: example

$(shell mkdir -p objs)

objs/main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

example: $(EXEC_OBJS) $(example_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@;

.PHONY: clean
clean:
	@rm -rf objs example
