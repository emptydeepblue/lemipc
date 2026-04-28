/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 00:00:00 by everonel          #+#    #+#             */
/*   Updated: 2026/04/23 00:00:00 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static void	dstr(char *b, int *l, const char *s)
{
	while (*s)
		b[(*l)++] = *s++;
}

static void	dint(char *b, int *l, int n)
{
	char	tmp[12];
	int		i;

	i = 0;
	if (n == 0)
		tmp[i++] = '0';
	while (n > 0)
	{
		tmp[i++] = '0' + (n % 10);
		n /= 10;
	}
	while (i > 0)
		b[(*l)++] = tmp[--i];
}

static void	draw_scores(t_board *board, char *b, int *l)
{
	int	cnt[MAX_TEAMS + 1];
	int	i;

	ft_memset(cnt, 0, sizeof(cnt));
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive && board->players[i].team >= 1
			&& board->players[i].team <= MAX_TEAMS)
			cnt[board->players[i].team]++;
		i++;
	}
	i = 1;
	while (i <= MAX_TEAMS)
	{
		if (cnt[i])
		{
			dstr(b, l, g_skins[i].color);
			dstr(b, l, g_skins[i].icon);
			dstr(b, l, TEXT_BOLD " T");
			dint(b, l, i);
			dstr(b, l, ":");
			dint(b, l, cnt[i]);
			dstr(b, l, RESET "  ");
		}
		i++;
	}
	dstr(b, l, "\n\n");
}

static void	draw_grid(t_board *board, char *b, int *l)
{
	int	grid[MAP_SIZE][MAP_SIZE];
	int	x;
	int	y;
	int	i;

	ft_memset(grid, 0, sizeof(grid));
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		x = board->players[i].pos.x;
		y = board->players[i].pos.y;
		if (board->players[i].alive && x >= 0 && x < MAP_SIZE
			&& y >= 0 && y < MAP_SIZE)
			grid[x][y] = board->players[i].team;
		i++;
	}
	y = 0;
	while (y < MAP_SIZE)
	{
		x = 0;
		while (x < MAP_SIZE)
		{
			if (grid[x][y])
			{
				dstr(b, l, g_skins[grid[x][y]].color);
				dstr(b, l, TEXT_BOLD);
				dint(b, l, grid[x][y]);
				dstr(b, l, RESET " ");
			}
			else
				dstr(b, l, "\xC2\xB7 ");
			x++;
		}
		dstr(b, l, "\n");
		y++;
	}
}

static void	render_frame(t_board *board)
{
	char	buf[MAP_SIZE * MAP_SIZE * 20 + 512];
	int		len;

	len = 0;
	dstr(buf, &len, "\033[H\033[2J");
	draw_scores(board, buf, &len);
	draw_grid(board, buf, &len);
	write(1, buf, len);
}

void	run_display(t_game *game)
{
	int	team;
	int	i;

	while (game->board->running)
	{
		render_frame(game->board);
		usleep(200000);
	}
	render_frame(game->board);
	team = 0;
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (game->board->players[i].alive)
		{
			team = game->board->players[i].team;
			break ;
		}
		i++;
	}
	if (team)
		draw_victory(team);
	else
		write(1, "Game over.\n", 11);
}
