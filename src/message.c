/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   message.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: everonel <everonel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 09:30:38 by everonel          #+#    #+#             */
/*   Updated: 2026/04/22 16:13:28 by everonel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lemipc.h"

void	send_target(int msgid, int team, t_position target)
{
    t_msgbuf msg;
    msg.mtype = team;
    *(t_position *)msg.mtext = target;
    if (msgsnd(msgid, &msg, sizeof(t_position), IPC_NOWAIT) < 0) {
        draw_error("Failed to send message");
    }
}

int    read_votes(int msgid, int team, t_position *votes, int max_votes)
{
    int count = 0;
    t_msgbuf msg;
    
    while (count < max_votes) {
        if (msgrcv(msgid, &msg, sizeof(t_position), team, IPC_NOWAIT) < 0) {
            if (errno == ENOMSG) {
                break;
            } else {
                draw_error("Failed to receive message");
                return -1;
            }
        }
        votes[count++] = *(t_position *)msg.mtext;
    }
    return count;
}

t_position vote_target(t_position *votes, int count, t_position current)
{
    int vote_counts[MAP_SIZE][MAP_SIZE] = {0};
    for (int i = 0; i < count; i++) {
        vote_counts[votes[i].x][votes[i].y]++;
    }
    t_position best = current;
    int best_count = 0;
    for (int x = 0; x < MAP_SIZE; x++) {
        for (int y = 0; y < MAP_SIZE; y++) {
            if (vote_counts[x][y] > best_count) {
                best_count = vote_counts[x][y];
                best.x = x;
                best.y = y;
            }
        }
    }
    return best;
}