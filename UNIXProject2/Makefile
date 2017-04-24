CC=gcc
CFLAGS=
DEPS = mysh_routines.h
OBJ = mysh.o mysh_routines.o myls.o mycp.o

make : $(OBJ)
	gcc -o mysh $^ $(CFLAGS)

mycat: mycat.c
	gcc -o mycat mycat.c
	
mycp: mycp.c
	gcc -o myls mycp.c

myls: myls.c
	gcc -o myls myls.c

mycat.o: mycat.c
	cc -c mycat.c
	
mycp.o: mycp.c
	cc -c mycp.c

myls.o: myls.c
	cc -c myls.c

mysh.o : mysh.c $(DEPS)
	cc -c mysh.c

mysh_routines.o : mysh_routines.c $(DEPS)
	cc -c mysh_routines.c

.PHONY: clean

clean:
	rm -f *.o mysh mycp myls mycat
