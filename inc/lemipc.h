#ifndef LEMIPC_H
# define LEMIPC_H
# include "../libft/libft.h"
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/types.h>
# include <unistd.h>
# include <errno.h>

# define MAP_SIZE    100
# define MAX_PLAYERS 4
# define MAX_TEAMS   4

# define SHM_KEY_ID  'L'
# define SEM_KEY_ID  'M'

# define RESET       "\033[0m"
# define COLOR_GREEN   "\033[32m"
# define COLOR_YELLOW  "\033[33m"
# define COLOR_MAGENTA "\033[35m"
# define COLOR_CYAN    "\033[36m"
# define TEXT_BOLD     "\033[1m"
# define COLOR_RED     "\033[31m"

typedef struct s_skin
{
	const char	*color;
	const char	*icon;
}	t_skin;

static const t_skin	g_skins[] = {
	[1] = {COLOR_MAGENTA, "🐙"},
	[2] = {COLOR_CYAN,    "🤖"},
	[3] = {COLOR_YELLOW,  "😺"},
	[4] = {COLOR_GREEN,   "👽"},
};

typedef enum e_direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
}	t_direction;

typedef struct s_position
{
	int	x;
	int	y;
}	t_position;

typedef struct s_move
{
	t_direction	direction;
	t_position	from;
	t_position	to;
}	t_move;

typedef struct s_player
{
	pid_t		pid;
	int			team;
	int			skin;
	int			alive;
	t_position	pos;
}	t_player;

typedef struct s_board
{
	t_player	players[MAX_PLAYERS * MAX_TEAMS];
	int			player_count;
	int			running;
}	t_board;

typedef struct s_game
{
	int			shmid;
	int			semid;
	t_board		*board;
}	t_game;

// game-manager.c
int			init_game(t_game *game);
void		cleanup_game(t_game *game);

// player-manager.c
t_player	register_player(t_board *board, int semid, int team);
void		sync_alive(t_player *player, t_board *board);

// moves-manager.c
int			move_player(t_player *player, t_board *board, int semid);

// draw.c
void		draw_spawn(t_player player);
void		draw_player(t_player player, t_move move);
void		draw_surroundings(t_player player);
void		draw_death(t_player player);
void		draw_victory(int team);
void		draw_error(char *message);

#endif
