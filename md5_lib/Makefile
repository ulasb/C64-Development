CC = cl65
AR = ar65
CFLAGS = -t c64

all: test.prg

md5.o: md5.c md5.h
	$(CC) $(CFLAGS) -c md5.c

md5.lib: md5.o
	$(AR) r md5.lib md5.o

test.prg: main.c md5.lib
	$(CC) $(CFLAGS) -o test.prg main.c md5.lib

clean:
	rm -f *.o *.lib *.prg
