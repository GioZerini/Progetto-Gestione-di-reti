CC=gcc
CFLAGS=-g -Wall -O -std=c99
LDLIBS=-lm -lrt

EXEC=main.out tool.out

all: $(EXEC)

%.out: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.o  
