EXE=allocate
CC=gcc
CFLAGS=-Wall
LIBS=-lm

$(EXE): main.c
	$(CC) $(CFLAGS) -o $(EXE) main.c $(LIBS)

clean:
	rm -f $(EXE) $(EXE).exe

.PHONY: clean
