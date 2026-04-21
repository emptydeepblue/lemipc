/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   player-manager.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 11:19:10 by everonel          #+#    #+#             */
/*   Updated: 2026/04/21 17:27:47 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static t_position	find_spawn(t_board *board)
{
	t_position	pos;
	int			step;

	step = MAP_SIZE / (MAX_PLAYERS * MAX_TEAMS + 1);
	pos.x = (board->player_count * step + step) % MAP_SIZE;
	pos.y = (board->player_count * step * 2 + step) % MAP_SIZE;
	return (pos);
}

static t_player	make_player(int team, t_position pos)
{
	t_player	p;

	ft_memset(&p, 0, sizeof(t_player));
	p.pid = getpid();
	p.team = team;
	p.skin = ((team - 1) % 4) + 1;
	p.alive = 1;
	p.pos = pos;
	return (p);
}

t_player	register_player(t_board *board, int semid, int team)
{
	t_player		empty;
	struct sembuf	lock;
	struct sembuf	unlock;
	int				i;

	ft_memset(&empty, 0, sizeof(t_player));
	lock = (struct sembuf){0, -1, 0};
	unlock = (struct sembuf){0, 1, 0};
	semop(semid, &lock, 1);
	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid == 0)
		{
			board->players[i] = make_player(team, find_spawn(board));
			board->player_count++;
			semop(semid, &unlock, 1);
			draw_spawn(board->players[i]);
			return (board->players[i]);
		}
		i++;
	}
	semop(semid, &unlock, 1);
	write(2, "Maximum number of players reached\n", 35);
	return (empty);
}

void	sync_alive(t_player *player, t_board *board)
{
	int	i;

	i = 0;
	while (i < MAX_PLAYERS * MAX_TEAMS)
	{
		if (board->players[i].pid == player->pid)
			return ;
		i++;
	}
	player->alive = 0;
}
