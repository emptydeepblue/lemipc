/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ipc.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 20:04:43 by everonel          #+#    #+#             */
/*   Updated: 2026/04/23 16:13:19 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

static int	get_shmid(int *is_creator)
{
	key_t	key;
	int		shmid;

	key = ftok("/tmp", SHM_KEY_ID);
	if (key < 0)
		return (-1);
	shmid = shmget(key, sizeof(t_board), IPC_CREAT | IPC_EXCL | 0666);
	if (shmid >= 0)
	{
		*is_creator = 1;
		return (shmid);
	}
	*is_creator = 0;
	return (shmget(key, sizeof(t_board), 0666));
}

static int	get_semid(int is_creator)
{
	key_t	key;
	int		semid;

	key = ftok("/tmp", SEM_KEY_ID);
	if (key < 0)
		return (-1);
	semid = semget(key, 1, IPC_CREAT | 0666);
	if (semid < 0)
		return (-1);
	if (is_creator)
		semctl(semid, 0, SETVAL, 1);
	return (semid);
}

static int	get_msgid(void)
{
	key_t	key;
	int		msgid;

	key = ftok("/tmp", MSG_KEY_ID);
	if (key < 0)
		return (-1);
	msgid = msgget(key, IPC_CREAT | 0666);
	if (msgid < 0)
		return (-1);
	return (msgid);
}

static void	drain_msgqueue(int msgid)
{
	t_msgbuf	tmp;

	while (msgrcv(msgid, &tmp, sizeof(tmp.mtext), 0, IPC_NOWAIT) >= 0)
		;
}

int		init_game(t_game *game)
{
	int			is_creator;

	ft_memset(game, 0, sizeof(t_game));
	game->shmid = get_shmid(&is_creator);
	if (game->shmid < 0) {
		draw_error("Failed to create or access shared memory");
		return (-1);
	}
	game->board = shmat(game->shmid, NULL, 0);
	if (game->board == (void *)-1) {
		draw_error("Failed to attach shared memory");
		return (-1);
	}
	game->is_creator = is_creator;
	if (is_creator)
	{
		ft_memset(game->board, 0, sizeof(t_board));
		game->board->running = 1;
	}
	game->semid = get_semid(is_creator);
	if (game->semid < 0)
	{
		draw_error("Failed to get semaphore ID");
		return (-1);
	}
	game->msgid = get_msgid();
	if (game->msgid < 0)
	{
		draw_error("Failed to get message queue ID");
		return (-1);
	}
	if (is_creator)
		drain_msgqueue(game->msgid);
	return (0);
}

int	init_display(t_game *game)
{
	key_t	key;

	ft_memset(game, 0, sizeof(t_game));
	key = ftok("/tmp", SHM_KEY_ID);
	if (key < 0)
		return (-1);
	game->shmid = shmget(key, sizeof(t_board), 0666);
	if (game->shmid < 0)
		return (-1);
	game->board = shmat(game->shmid, NULL, SHM_RDONLY);
	if (game->board == (void *)-1)
		return (-1);
	return (0);
}

void	cleanup_game(t_game *game)
{
	int		game_over;

	if (!game->board)
		return ;
	game_over = !game->board->running;
	shmdt(game->board);
	if (!game_over)
		return ;
	shmctl(game->shmid, IPC_RMID, NULL);
	semctl(game->semid, 0, IPC_RMID);
	msgctl(game->msgid, IPC_RMID, NULL);
}

