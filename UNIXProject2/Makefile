CC=gcc
CFLAGS=
DEPS = mysh_routines.h
OBJ = mysh.o mysh_routines.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	gcc -o mysh $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o mysh
