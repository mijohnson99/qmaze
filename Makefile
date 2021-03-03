CC = clang
CFLAGS = -Os -g

all: game

game: game.o maze.o
	$(CC) $(CFLAGS) $^ -o $@

maze: src/maze.c src/*.h
	$(CC) $(CFLAGS) -DUNIT_TEST $< -o $@

%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f maze game *.o
