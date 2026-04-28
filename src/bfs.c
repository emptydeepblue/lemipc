/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bfs.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 10:00:00 by everonel          #+#    #+#             */
/*   Updated: 2026/04/23 10:00:00 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static int	is_blocked(t_board *board, t_position p, t_position allow)
{
	int	i;

	if (p.x == allow.x && p.y == allow.y)
		return (0);
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].alive
			&& board->players[i].pos.x == p.x
			&& board->players[i].pos.y == p.y)
			return (1);
		i++;
	}
	return (0);
}

static void	init_dist(int dist[MAP_SIZE][MAP_SIZE])
{
	int	x;
	int	y;

	x = 0;
	while (x < MAP_SIZE)
	{
		y = 0;
		while (y < MAP_SIZE)
			dist[x][y++] = -1;
		x++;
	}
}

static void	expand(t_board *board, t_position cur, t_position target,
			int dist[MAP_SIZE][MAP_SIZE], t_position *queue, int *tail)
{
	static const int	dx[4] = {0, 0, -1, 1};
	static const int	dy[4] = {-1, 1, 0, 0};
	t_position			nx;
	int					i;

	i = -1;
	while (++i < 4)
	{
		nx.x = cur.x + dx[i];
		nx.y = cur.y + dy[i];
		if (nx.x < 0 || nx.x >= MAP_SIZE || nx.y < 0 || nx.y >= MAP_SIZE)
			continue ;
		if (dist[nx.x][nx.y] != -1)
			continue ;
		if (is_blocked(board, nx, target))
			continue ;
		dist[nx.x][nx.y] = dist[cur.x][cur.y] + 1;
		queue[(*tail)++] = nx;
	}
}

void	bfs_fill(t_board *board, t_position target,
		int dist[MAP_SIZE][MAP_SIZE])
{
	t_position	queue[MAP_SIZE * MAP_SIZE];
	int			head;
	int			tail;

	init_dist(dist);
	if (target.x < 0 || target.x >= MAP_SIZE
		|| target.y < 0 || target.y >= MAP_SIZE)
		return ;
	dist[target.x][target.y] = 0;
	head = 0;
	tail = 0;
	queue[tail++] = target;
	while (head < tail)
	{
		expand(board, queue[head], target, dist, queue, &tail);
		head++;
	}
}
