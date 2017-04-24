CC=gcc
CFLAGS=
DEPS = mysh_routines.h
OBJ = mysh.o mysh_routines.o

make : $(OBJ)
	gcc -o mysh $^ $(CFLAGS)

mysh.o : mysh.c $(DEPS)
	cc -c mysh.c

mysh_routines.o : mysh_routines.c $(DEPS)
	cc -c mysh_routines.c

.PHONY: clean

clean:
	rm -f *.o mysh
