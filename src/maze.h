#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct maze {
	int width, height;
	char tile[];
};

struct maze *new_maze(int, int);
#define free_maze(m) free(m)
void maze_initialize(struct maze *);
void maze_generate(struct maze *);

struct maze *new_maze(int, int);
void maze_initialize(struct maze *);
int rand_neighbor(struct maze *, int, char);
#define wall(p1, p2) ((p1)+((p2)-(p1))/2)
void maze_generate(struct maze *);
void maze_display(struct maze *);
int main(int, char **);
