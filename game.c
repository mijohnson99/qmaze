#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "src/aterm.h"
#include "src/maze.h"

struct game {
	struct maze *maze;
	int visibility;
	int player_x;
	int player_y;
};

#define ABS(x) ((x)<0?-(x):(x))
bool is_visible(struct game *g, int x, int y)
{
	int dx = x - g->player_x;
	int dy = y - g->player_y;
	// Perimeter should be visible
	if (x == 0 || y == 0)
		return true;
	if (x == g->maze->width-1 || y == g->maze->height-1)
		return true;
	// Things near player should be visible
	if (ABS(dx) > g->visibility)
		return false;
	if (ABS(dy) > g->visibility)
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
		printf(SGR(RESET)"\n\r"); // Carriage return necessary in raw mode
	}
}

void game_exit(int sig)
{
	// Fix the terminal
	system("stty cooked"); // TODO: Use termios

	if (sig == SIGTERM)
		exit(0);
	else
		exit(sig);
	// SIGTERM is not an error, so return 0
	exit(sig == SIGTERM ? 0 : sig);
}

void try_move(struct game *g, char key)
{
	int dx = 0, dy = 0;
	int new_pos;
	int n = key - '0';
	if (key < 0 || key == 4 || key == 'q') // Quit action: q or ^D
		game_exit(SIGTERM);
	if ('1' <= key && key <= '9') { // Movement: 1-9 (TODO: vi keys)
		dx =   (n-1) % 3 - 1;
		dy = -((n-1) / 3 - 1);
	}
	// The only open squares are at the corners.
	// Therefore, only need to range check twice.
	if (dy < 0 && g->player_y == 0)
		return; // TODO: Exit condition
	if (dy > 0 && g->player_y == g->maze->height-1)
		return; // TODO: Win condition
	// Calculate moved position
	new_pos = (g->player_x + dx) + (g->player_y + dy) * g->maze->width;
	// If we bump into a wall, don't move
	if (g->maze->tile[new_pos] == '#')
		return;
	// Otherwise, move
	g->player_x += dx;
	g->player_y += dy;
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
	g.maze = new_maze(w, h); // TODO: Is having a struct for this really necessary?
	maze_generate(g.maze);
	g.visibility = 5;
	// Place the player
	g.player_x = 1;
	g.player_y = 0;

	// Install signal handlers before messing with the terminal
	signal(SIGTERM, game_exit);
	signal(SIGINT, game_exit);
	signal(SIGHUP, game_exit);
	signal(SIGSEGV, game_exit);
	system("stty raw"); // TODO: Use termios
	// Game loop
	for (;;) {
		draw_game(&g);
		try_move(&g, getchar());
	}
	// Unreachable
	return 0;
}
