CC=gcc
CFLAGS=-I.

SPAWN = spawnMain.c

SPAWNEXE = spawn

DEPS = spawnFunctions.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

spawn: $(SPAWN)
	$(CC) -o $(SPAWNEXE) $(SPAWN)

clean:
	rm -f *.o
