/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   draw.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:12:20 by everonel          #+#    #+#             */
/*   Updated: 2026/04/21 12:19:52 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static void	buf_add_str(char *buf, int *len, const char *str)
{
	while (*str)
		buf[(*len)++] = *str++;
}

static void	buf_add_int(char *buf, int *len, int n)
{
	char	tmp[12];
	int		i;
	int		neg;

	i = 0;
	neg = (n < 0);
	if (neg)
		n = -n;
	if (n == 0)
		tmp[i++] = '0';
	while (n > 0)
	{
		tmp[i++] = '0' + (n % 10);
		n /= 10;
	}
	if (neg)
		tmp[i++] = '-';
	while (i > 0)
		buf[(*len)++] = tmp[--i];
}

static void	buf_add_prefix(char *buf, int *len, t_player player)
{
	buf_add_str(buf, len, g_skins[player.skin].color);
	buf_add_str(buf, len, g_skins[player.skin].icon);
	buf_add_str(buf, len, " " TEXT_BOLD "t");
	buf_add_int(buf, len, player.team);
	buf_add_str(buf, len, " P");
	buf_add_int(buf, len, player.pid);
	buf_add_str(buf, len, RESET ": ");
}

void	draw_spawn(t_player player)
{
	char	buf[128];
	int		len;

	len = 0;
	buf_add_prefix(buf, &len, player);
	buf_add_str(buf, &len, "spawned at (");
	buf_add_int(buf, &len, player.pos.x);
	buf_add_str(buf, &len, ", ");
	buf_add_int(buf, &len, player.pos.y);
	buf_add_str(buf, &len, ")\n");
	write(1, buf, len);
}

void	draw_player(t_player player, t_move move)
{
	char	buf[128];
	int		len;

	len = 0;
	buf_add_prefix(buf, &len, player);
	buf_add_str(buf, &len, "moved (");
	buf_add_int(buf, &len, move.from.x);
	buf_add_str(buf, &len, ",");
	buf_add_int(buf, &len, move.from.y);
	buf_add_str(buf, &len, ") -> (");
	buf_add_int(buf, &len, move.to.x);
	buf_add_str(buf, &len, ",");
	buf_add_int(buf, &len, move.to.y);
	buf_add_str(buf, &len, ")\n");
	write(1, buf, len);
}

void	draw_surroundings(t_player player)
{
	char	buf[128];
	int		len;

	len = 0;
	buf_add_prefix(buf, &len, player);
	buf_add_str(buf, &len, "surrounded at (");
	buf_add_int(buf, &len, player.pos.x);
	buf_add_str(buf, &len, ", ");
	buf_add_int(buf, &len, player.pos.y);
	buf_add_str(buf, &len, ")\n");
	write(1, buf, len);
}

void	draw_death(t_player player)
{
	char	buf[128];
	int		len;

	len = 0;
	buf_add_prefix(buf, &len, player);
	buf_add_str(buf, &len, "eliminated at (");
	buf_add_int(buf, &len, player.pos.x);
	buf_add_str(buf, &len, ", ");
	buf_add_int(buf, &len, player.pos.y);
	buf_add_str(buf, &len, ")\n");
	write(1, buf, len);
}

void	draw_victory(int team)
{
	char	buf[64];
	int		len;

	len = 0;
	buf_add_str(buf, &len, TEXT_BOLD "Team ");
	buf_add_int(buf, &len, team);
	buf_add_str(buf, &len, " wins!" RESET "\n");
	write(1, buf, len);
}

void	draw_error(char *message)
{
	char	buf[256];
	int		len;

	len = 0;
	buf_add_str(buf, &len, TEXT_BOLD COLOR_RED "Error: ");
	buf_add_str(buf, &len, message);
	buf_add_str(buf, &len, RESET "\n");
	write(2, buf, len);
}
