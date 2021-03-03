#include <stdio.h>
#include <stdlib.h>
#include "src/aterm.h"
#include "src/maze.h"

struct game {
	struct maze *maze;
	int player_x;
	int player_y;
};

bool is_visible(struct game *g, int x, int y)
{
	int dx = x - g->player_x;
	int dy = y - g->player_y;
	if (!(-5 <= dx && dx <= 5))
		return false;
	if (!(-5 <= dy && dy <= 5))
		return false;
	// TODO: Make long corridors visible
	return true;
}

void draw_game(struct game *g)
{
	// Reset cursor to top left corner
	printf(CUP("1","1"));
	for (int y = 0; y < g->maze->height; y++) {
		for (int x = 0; x < g->maze->width; x++) {
			int pos = x + y * g->maze->width;
			// If it's the player, draw them
			if (x == g->player_x && y == g->player_y) {
				printf(SGR(BG_COLR(BLACK))"@");
				continue;
			}
			// If it's not visible, draw "nothing"
			if (!is_visible(g, x, y)) {
				printf(SGR(BG_COLR(BLACK))"?");
				continue;
			}
			// Otherwise, draw the corresponding glyph
			if (g->maze->tile[pos] == '#')
				printf(SGR(BG_COLR(WHITE))" ");
			else
				printf(SGR(BG_COLR(BLACK))" ");
		}
		printf(SGR(RESET)"\n");
	}
}

int main(int argc, char **argv)
{
	int w = 80, h = 24;
	struct game g;
	printf(SGR(RESET) CLS); // Clear screen
	// Accept size parameters
	if (argc > 2) {
		w = atoi(argv[1]);
		h = atoi(argv[2]);
	}
	// Generate the maze
	g.maze = new_maze(w, h);
	maze_generate(g.maze);
	// Place the player
	g.player_x = 1;
	g.player_y = 0;

	// Draw the screen
	draw_game(&g);

	return 0;
}
