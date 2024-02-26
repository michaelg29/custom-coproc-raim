
LOG_FILE ?= PCSpim.log
OUT_FILE ?= raim.bin

EXE      ?= system
CC       ?= gcc
DEPS     = $(wildcard *.h)
SRCS     = $(wildcard *.c)
OBJS     = $(SRCS:.c=.o)

%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS) $(IFLAGS) $(DEFINE_FLAGS)

$(EXE): $(OBJS)
	$(CC) -g -o $@ $^ $(CFLAGS) $(LFLAGS)

run: $(EXE)
	./$(EXE) $(LOG_FILE) $(OUT_FILE)

clean:
	$(RM) $(EXE) $(OBJS)
