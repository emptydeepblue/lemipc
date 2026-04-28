/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lemipc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:09:30 by everonel          #+#    #+#             */
/*   Updated: 2026/04/23 16:13:02 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static int	display_mode(void)
{
	t_game	game;

	while (init_display(&game) < 0)
		usleep(200000);
	run_display(&game);
	shmdt(game.board);
	return (0);
}

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
	if (team == 0)
		return (display_mode());
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
		sync_alive(&player, game.board, game.semid);
		if (!player.alive)
			break ;
		if (move_player(&player, game.board, game.semid, game.msgid))
		{
			sync_alive(&player, game.board, game.semid);
			if (player.alive)
				draw_victory(player.team);
			break ;
		}
		usleep(200000);
	}
	cleanup_game(&game);
	return (0);
}
