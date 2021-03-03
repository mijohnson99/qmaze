#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "src/aterm.h"
#include "src/maze.h"

struct game {
	struct maze *maze;
	struct maze *superposition;
	double volatility;
	int player_x;
	int player_y;
};

#define ABS(x) ((x)<0?-(x):(x))
bool is_visible(struct game *g, int x, int y)
{
	int dx = x - g->player_x;
	int dx_dir = dx < 0 ? -1 : 1;
	int dy = y - g->player_y;
	int dy_dir = dy < 0 ? -1 : 1;
	// Player can see themself
	if (dx == 0 && dy == 0)
		return true;
	// Something cannot be seen past a wall
	if (dy == 0 && g->maze->tile[x-dx_dir + y * g->maze->width] == '#')
		return false;
	if (dx == 0 && g->maze->tile[x + (y-dy_dir) * g->maze->width] == '#')
		return false;
	// Player can see down corridors
	if (dy == 0)
		return is_visible(g, x-dx_dir, y);
	if (dx == 0)
		return is_visible(g, x, y-dy_dir);
	// Player can see walls of corridors
	if (dy == -1 || dy == 1)
		return is_visible(g, x, y-dy_dir);
	if (dx == -1 || dx == 1)
		return is_visible(g, x-dx_dir, y);
	return false;
}

bool is_perimeter(struct game *g, int x, int y)
{
	if (x == 0 || y == 0)
		return true;
	if (x == g->maze->width-1 || y == g->maze->height-1)
		return true;
	return false;
}

void draw_game(struct game *g)
{
	// Reset cursor to top left corner
	printf(CUP("1","1"));
	for (int y = 0; y < g->maze->height; y++) {
		printf(SGR(FG_COLR(WHITE)));
		for (int x = 0; x < g->maze->width; x++) {
			int pos = x + y * g->maze->width;
			// If it's the player, draw them
			if (x == g->player_x && y == g->player_y) {
				printf(SGR(BG_COLR(BLACK))"@");
				continue;
			}
			// If it's not visible, draw "nothing"
			if (!is_visible(g, x, y) && !is_perimeter(g, x, y)) {
				printf(SGR(BG_COLR(BLACK))"?");
				continue;
			}
			// Otherwise, draw the corresponding glyph
			if (g->maze->tile[pos] == '#')
				printf(SGR(BG_COLR(WHITE))"#");
			else
				printf(SGR(BG_COLR(BLACK))" ");
		}
		printf(SGR(RESET)"\r\n"); // Carriage return is necessary in raw mode
	}
}

void try_move(struct game *g, char key)
{
	int dx = 0, dy = 0;
	int new_pos;
	int n = key - '0';
	if (key < 0 || key == 4 || key == 'q') // Quit action: q or ^D
		raise(SIGTERM);
	if ('1' <= key && key <= '9') { // Movement: 1-9 (TODO: vi keys)
		dx =   (n-1) % 3 - 1;
		dy = -((n-1) / 3 - 1);
	}
	// The only open squares are at the corners.
	// Therefore, only need to range check twice.
	if (dy < 0 && g->player_y == 0) {
		printf("You have exited the labyrinth a coward.\r\n");
		raise(SIGTERM);
	}
	if (dy > 0 && g->player_y == g->maze->height-1) {
		puts("You have conquered the labyrinth!\r\n");
		raise(SIGTERM);
	}
	// Calculate moved position
	new_pos = (g->player_x + dx) + (g->player_y + dy) * g->maze->width;
	// If we bump into a wall, don't move
	if (g->maze->tile[new_pos] == '#')
		return;
	// Otherwise, move
	g->player_x += dx;
	g->player_y += dy;
}

void game_exit(int sig)
{
	// Fix the terminal
	system("stty sane"); // TODO: Use termios
	printf(CUS); // Show cursor

	exit(sig == SIGTERM ? 0 : sig); // SIGTERM is not an error
}

void mutate(struct game *g)
{
	// Measure the superposition
	maze_initialize(g->superposition);
	maze_generate(g->superposition);
	// Apply it to tiles that the player can't see
	for (int y = 0; y < g->maze->height; y++) {
		for (int x = 0; x < g->maze->width; x++) {
			int pos = x + y * g->maze->width;
			if (!is_visible(g, x, y))
				g->maze->tile[pos] = g->superposition->tile[pos];
		}
	}
}

int main(int argc, char **argv)
{
	int w = 80, h = 24;
	struct game g;
	srand(time(NULL));
	printf(CUH SGR(RESET) CLS); // Hide cursor and clear screen
	// Accept size parameters
	if (argc > 2) {
		w = atoi(argv[1]);
		h = atoi(argv[2]);
	}
	// Generate the maze
	g.maze = new_maze(w, h); // TODO: Is having struct game really necessary?
	maze_initialize(g.maze);
	maze_generate(g.maze);
	g.superposition = new_maze(w, h); // Allocate for the superposition layer
	g.volatility = 0.1; // Percent chance to mutate the labyrinth
	// Place the player
	g.player_x = 1;
	g.player_y = 0;

	// Install signal handlers before messing with the terminal
	signal(SIGTERM, game_exit);
	signal(SIGINT, game_exit);
	signal(SIGHUP, game_exit);
	signal(SIGSEGV, game_exit);
	system("stty raw -echo"); // TODO: Use termios
	// Game loop
	for (;;) {
		draw_game(&g);
		try_move(&g, getchar());
		if ((double)rand() / RAND_MAX < g.volatility)
			mutate(&g);
	}
	// Unreachable
	return 0;
}
