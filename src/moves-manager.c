/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   moves-manager.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 17:00:00 by everonel          #+#    #+#             */
/*   Updated: 2026/04/21 17:27:47 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static int	manhattan(t_position a, t_position b)
{
	int	dx;
	int	dy;

	dx = a.x - b.x;
	dy = a.y - b.y;
	return ((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
}

static t_position	find_target(t_player *player, t_board *board)
{
	t_position	best;
	int			best_dist;
	int			dist;
	int			i;

	best = player->pos;
	best_dist = -1;
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive && board->players[i].team != player->team)
		{
			dist = manhattan(player->pos, board->players[i].pos);
			if (best_dist < 0 || dist < best_dist)
			{
				best_dist = dist;
				best = board->players[i].pos;
			}
		}
		i++;
	}
	return (best);
}

static t_position	step_toward(t_position from, t_position to)
{
	t_position	next;
	int			dx;
	int			dy;

	next = from;
	if (from.x == to.x && from.y == to.y)
		return (next);
	dx = to.x - from.x;
	dy = to.y - from.y;
	if ((dx < 0 ? -dx : dx) >= (dy < 0 ? -dy : dy))
		next.x += (dx > 0) ? 1 : -1;
	else
		next.y += (dy > 0) ? 1 : -1;
	return (next);
}

static int	surviving_team(t_board *board)
{
	int	team;
	int	i;

	team = 0;
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive)
		{
			if (team == 0)
				team = board->players[i].team;
			else if (board->players[i].team != team)
				return (0);
		}
		i++;
	}
	return (team);
}

static int	handle_collision(t_player *player, t_board *board,
				t_position pos, t_player *killed)
{
	int	i;

	ft_memset(killed, 0, sizeof(t_player));
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid != 0
			&& board->players[i].pid != player->pid
			&& board->players[i].pos.x == pos.x
			&& board->players[i].pos.y == pos.y)
		{
			if (board->players[i].team == player->team)
				return (0);
			*killed = board->players[i];
			board->players[i].alive = 0;
			board->players[i].pid = 0;
			board->player_count--;
			if (surviving_team(board))
				board->running = 0;
			return (1);
		}
		i++;
	}
	return (1);
}

static void	update_board_pos(t_player *player, t_board *board, t_position pos)
{
	int	i;

	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid == player->pid)
		{
			board->players[i].pos = pos;
			return ;
		}
		i++;
	}
}

int	move_player(t_player *player, t_board *board, int semid)
{
	struct sembuf	lock;
	struct sembuf	unlock;
	t_position		new_pos;
	t_move			move;
	t_player		killed;

	lock = (struct sembuf){0, -1, 0};
	unlock = (struct sembuf){0, 1, 0};
	new_pos = step_toward(player->pos, find_target(player, board));
	if (new_pos.x == player->pos.x && new_pos.y == player->pos.y)
		return (0);
	semop(semid, &lock, 1);
	if (!handle_collision(player, board, new_pos, &killed))
		return (semop(semid, &unlock, 1), 0);
	move = (t_move){0, player->pos, new_pos};
	update_board_pos(player, board, new_pos);
	player->pos = new_pos;
	semop(semid, &unlock, 1);
	if (killed.alive)
		draw_death(killed);
	draw_player(*player, move);
	return (!board->running);
}
