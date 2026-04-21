/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   draw.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:12:20 by everonel          #+#    #+#             */
/*   Updated: 2026/04/21 11:27:44 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static void	put_prefix(t_player player)
{
	ft_putstr_fd((char *)g_skins[player.skin].color, 1);
	ft_putstr_fd((char *)g_skins[player.skin].icon, 1);
	ft_putstr_fd(" " TEXT_BOLD "t", 1);
	ft_putnbr_fd(player.team, 1);
	ft_putstr_fd(" P", 1);
	ft_putnbr_fd(player.pid, 1);
	ft_putstr_fd(RESET ": ", 1);
}

void	draw_spawn(t_player player)
{
	put_prefix(player);
	ft_putstr_fd("spawned at (", 1);
	ft_putnbr_fd(player.pos.x, 1);
	ft_putstr_fd(", ", 1);
	ft_putnbr_fd(player.pos.y, 1);
	ft_putstr_fd(")\n", 1);
}

void	draw_player(t_player player, t_move move)
{
	put_prefix(player);
	ft_putstr_fd("moved (", 1);
	ft_putnbr_fd(move.from.x, 1);
	ft_putstr_fd(",", 1);
	ft_putnbr_fd(move.from.y, 1);
	ft_putstr_fd(") -> (", 1);
	ft_putnbr_fd(move.to.x, 1);
	ft_putstr_fd(",", 1);
	ft_putnbr_fd(move.to.y, 1);
	ft_putstr_fd(")\n", 1);
}

void	draw_surroundings(t_player player)
{
	put_prefix(player);
	ft_putstr_fd("surrounded at (", 1);
	ft_putnbr_fd(player.pos.x, 1);
	ft_putstr_fd(", ", 1);
	ft_putnbr_fd(player.pos.y, 1);
	ft_putstr_fd(")\n", 1);
}

void	draw_death(t_player player)
{
	put_prefix(player);
	ft_putstr_fd("eliminated at (", 1);
	ft_putnbr_fd(player.pos.x, 1);
	ft_putstr_fd(", ", 1);
	ft_putnbr_fd(player.pos.y, 1);
	ft_putstr_fd(")\n", 1);
}

void	draw_victory(int team)
{
	ft_putstr_fd(TEXT_BOLD "Team ", 1);
	ft_putnbr_fd(team, 1);
	ft_putstr_fd(" wins!" RESET "\n", 1);
}

void draw_error(char *message)
{
	ft_putstr_fd(TEXT_BOLD COLOR_RED "Error: ", 2);
	ft_putstr_fd(message, 2);
	ft_putstr_fd(RESET "\n", 2);
}