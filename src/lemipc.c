/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lemipc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:09:30 by everonel          #+#    #+#             */
/*   Updated: 2026/04/22 12:23:56 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

int	main(int ac, char **av)
{
	int			team;
	t_game		game;
	t_player	player;

	if (ac != 2)
	{
		draw_error("Usage: ./lemipc <team_number>");
		return (1);
	}
	team = ft_atoi(av[1]);
	if (team < 1 || team > MAX_TEAMS)
	{
		draw_error("Team number must be between 1 and MAX_TEAMS");
		return (1);
	}
	if (init_game(&game) < 0)
		return (1);
	player = register_player(game.board, game.semid, team);
	if (player.pid <= 0)
	{
		draw_error("Failed to register player");
		cleanup_game(&game);
		return (1);
	}
	while (game.board->running && player.alive)
	{
		sync_alive(&player, game.board);
		if (!player.alive)
			break ;
		if (move_player(&player, game.board, game.semid, game.msgid))
		{
			draw_victory(player.team);
			break ;
		}
		sleep(1);
	}
	cleanup_game(&game);
	return (0);
}
