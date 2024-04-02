
LOG_FILE ?= PCSpim.log
OUT_FILE ?= raim.bin

EXE      ?= s2bin
CC       ?= gcc
CXX      ?= g++
DEPS     = ../../model/include/isa.h
SRCS     = $(wildcard *.c)
OBJS     = $(wildcard *.o)

IFLAGS   = -I../../model/include

s2bin.o: s2bin.c $(DEPS)
	$(CXX) $(IFLAGS) -g -c -o $@ $<

isa.o: ../../model/src/isa.cpp $(DEPS)
	$(CXX) $(IFLAGS) -g -c -o $@ $<

$(EXE): s2bin.o isa.o
	$(CXX) -g -o $@ $^

run: $(EXE)
	./$(EXE) $(LOG_FILE) $(OUT_FILE)

clean:
	$(RM) $(EXE) $(OBJS)
