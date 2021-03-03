#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct maze {
	int width, height;
	char tile[];
};

struct maze *new_maze(int, int);
void maze_generate(struct maze *);
