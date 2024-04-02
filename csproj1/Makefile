CC = gcc
CFLAGS = -Wall
EXE = allocate

all: $(EXE)

$(EXE): main.c
	$(CC) $(CFLAGS) -o $(EXE) $< -lm

%.o: %.c %.h
	$(CC)·-c -o $@ $< $(CC) $(CFLAGS)

clean:
	rm -f *.o $(EXE)

format:
	clang-format -i *.c *.h