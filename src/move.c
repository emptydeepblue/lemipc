/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   move.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 17:26:32 by everonel          #+#    #+#             */
/*   Updated: 2026/04/23 10:00:00 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static t_position	g_prev = {-1, -1};
static int			g_turn = 0;

static int	manhattan(t_position a, t_position b)
{
	int	dx;
	int	dy;

	dx = a.x - b.x;
	dy = a.y - b.y;
	return ((dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy));
}

static t_position	team_centroid(t_player *player, t_board *board)
{
	t_position	c;
	int			n;
	int			i;

	c = (t_position){0, 0};
	n = 0;
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive && board->players[i].team == player->team)
		{
			c.x += board->players[i].pos.x;
			c.y += board->players[i].pos.y;
			n++;
		}
		i++;
	}
	if (n > 0)
	{
		c.x /= n;
		c.y /= n;
	}
	else
		c = player->pos;
	return (c);
}

static t_position	find_nearest(t_player *player, t_board *board)
{
	t_position	from;
	t_position	best;
	int			best_dist;
	int			dist;
	int			i;

	from = team_centroid(player, board);
	best = player->pos;
	best_dist = -1;
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive && board->players[i].team != player->team)
		{
			dist = manhattan(from, board->players[i].pos);
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

static int	handle_collision(t_player *player, t_board *board, t_position pos)
{
	int	i;

	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid != 0
			&& board->players[i].pid != player->pid
			&& board->players[i].pos.x == pos.x
			&& board->players[i].pos.y == pos.y)
			return (0);
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

static int	team_at_pos(t_board *board, t_position pos, int self_team)
{
	int	i;

	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive
			&& board->players[i].team != self_team
			&& board->players[i].pos.x == pos.x
			&& board->players[i].pos.y == pos.y)
			return (board->players[i].team);
		i++;
	}
	return (0);
}

static int	is_pos_surrounded(t_board *board, t_position pos, int team)
{
	t_position	adj[4];
	int			team_count[MAX_TEAMS + 1];
	int			i;

	adj[0] = (t_position){pos.x, pos.y - 1};
	adj[1] = (t_position){pos.x, pos.y + 1};
	adj[2] = (t_position){pos.x - 1, pos.y};
	adj[3] = (t_position){pos.x + 1, pos.y};
	ft_memset(team_count, 0, sizeof(team_count));
	i = 0;
	while (i < 4)
		team_count[team_at_pos(board, adj[i++], team)]++;
	i = 1;
	while (i <= MAX_TEAMS)
	{
		if (team_count[i++] >= 2)
			return (1);
	}
	return (0);
}

static int	is_surrounded(t_player *player, t_board *board)
{
	return (is_pos_surrounded(board, player->pos, player->team));
}

static void	eliminate_slot(t_board *board, int slot)
{
	board->players[slot].alive = 0;
	board->players[slot].pid = 0;
	board->player_count--;
}

static int	eliminate_self(t_player *player, t_board *board)
{
	int	winner;
	int	i;

	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid == player->pid)
		{
			eliminate_slot(board, i);
			break ;
		}
		i++;
	}
	winner = surviving_team(board);
	if (winner)
		board->running = 0;
	return (winner);
}

static t_position	get_voted_target(t_player *player, t_board *board,
					int msgid)
{
	t_position	votes[8];
	t_position	mine;
	t_position	chosen;
	int			count;
	int			i;

	mine = find_nearest(player, board);
	count = read_votes(msgid, player->team, votes, 8);
	if (count < 8)
		votes[count++] = mine;
	chosen = vote_target(votes, count, mine);
	i = 0;
	while (i < count)
	{
		send_target(msgid, player->team, votes[i]);
		i++;
	}
	return (chosen);
}

static void	fill_adj(t_player *player, t_position *adj)
{
	adj[0] = (t_position){player->pos.x - 1, player->pos.y};
	adj[1] = (t_position){player->pos.x + 1, player->pos.y};
	adj[2] = (t_position){player->pos.x, player->pos.y - 1};
	adj[3] = (t_position){player->pos.x, player->pos.y + 1};
}

static int	adj_ok(t_player *player, t_board *board, t_position p, int allow_prev)
{
	if (p.x < 0 || p.x >= MAP_SIZE || p.y < 0 || p.y >= MAP_SIZE)
		return (0);
	if (!handle_collision(player, board, p))
		return (0);
	if (!allow_prev && p.x == g_prev.x && p.y == g_prev.y)
		return (0);
	return (1);
}

static t_position	pick_by_dist(t_player *player, t_board *board,
					int dist[MAP_SIZE][MAP_SIZE])
{
	t_position	adj[4];
	t_position	ties[4];
	int			best;
	int			nties;
	int			i;

	fill_adj(player, adj);
	best = -1;
	nties = 0;
	i = -1;
	while (++i < 4)
	{
		if (!adj_ok(player, board, adj[i], 0))
			continue ;
		if (dist[adj[i].x][adj[i].y] < 0)
			continue ;
		if (best < 0 || dist[adj[i].x][adj[i].y] < best)
		{
			best = dist[adj[i].x][adj[i].y];
			nties = 0;
		}
		if (dist[adj[i].x][adj[i].y] == best)
			ties[nties++] = adj[i];
	}
	if (nties == 0)
		return (player->pos);
	return (ties[g_turn++ % nties]);
}

static t_position	pick_any_free(t_player *player, t_board *board,
					int allow_prev)
{
	t_position	adj[4];
	t_position	ties[4];
	int			nties;
	int			i;

	fill_adj(player, adj);
	nties = 0;
	i = -1;
	while (++i < 4)
	{
		if (!adj_ok(player, board, adj[i], allow_prev))
			continue ;
		ties[nties++] = adj[i];
	}
	if (nties == 0)
		return (player->pos);
	return (ties[g_turn++ % nties]);
}

static t_position	choose_step(t_player *player, t_board *board,
					t_position target)
{
	int			dist[MAP_SIZE][MAP_SIZE];
	t_position	step;

	bfs_fill(board, target, dist);
	step = pick_by_dist(player, board, dist);
	if (step.x == player->pos.x && step.y == player->pos.y)
		step = pick_any_free(player, board, 0);
	if (step.x == player->pos.x && step.y == player->pos.y)
		step = pick_any_free(player, board, 1);
	return (step);
}

static int	do_move(t_player *player, t_board *board, int semid, int msgid)
{
	struct sembuf	lock;
	struct sembuf	unlock;
	t_position		target;
	t_position		new_pos;
	t_move			move;

	lock = (struct sembuf){0, -1, 0};
	unlock = (struct sembuf){0, 1, 0};
	semop(semid, &lock, 1);
	target = get_voted_target(player, board, msgid);
	new_pos = choose_step(player, board, target);
	if (new_pos.x == player->pos.x && new_pos.y == player->pos.y)
		return (semop(semid, &unlock, 1), 0);
	move = (t_move){0, player->pos, new_pos};
	update_board_pos(player, board, new_pos);
	g_prev = player->pos;
	player->pos = new_pos;
	semop(semid, &unlock, 1);
	draw_player(*player, move);
	return (!board->running);
}

int	move_player(t_player *player, t_board *board, int semid, int msgid)
{
	struct sembuf	lock;
	struct sembuf	unlock;
	int				winner;

	lock = (struct sembuf){0, -1, 0};
	unlock = (struct sembuf){0, 1, 0};
	semop(semid, &lock, 1);
	if (is_surrounded(player, board))
	{
		winner = eliminate_self(player, board);
		semop(semid, &unlock, 1);
		draw_surroundings(*player);
		player->alive = 0;
		if (winner)
			draw_victory(winner);
		return (!board->running);
	}
	semop(semid, &unlock, 1);
	return (do_move(player, board, semid, msgid));
}
