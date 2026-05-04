/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:43:05 by fatemeh           #+#    #+#             */
/*   Updated: 2026/05/04 17:26:53 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Lock two dongles in a consistent order to avoid deadlock.
 *
 * This function locks the dongles by address order so that all threads
 * lock in the same order. If both pointers are equal, it locks once.
 *
 * @param a First dongle.
 * @param b Second dongle.
 */
static void	lock_pair(t_dongle *a, t_dongle *b)
{
	if (a == b)
		pthread_mutex_lock(&a->mutex);
	else if (a < b)
	{
		pthread_mutex_lock(&a->mutex);
		pthread_mutex_lock(&b->mutex);
	}
	else
	{
		pthread_mutex_lock(&b->mutex);
		pthread_mutex_lock(&a->mutex);
	}
}

/**
 * @brief Unlock two dongles locked by lock_pair.
 *
 * If both pointers are equal, it unlocks once.
 *
 * @param a First dongle.
 * @param b Second dongle.
 */
static void	unlock_pair(t_dongle *a, t_dongle *b)
{
	if (a == b)
		pthread_mutex_unlock(&a->mutex);
	else if (a < b)
	{
		pthread_mutex_unlock(&b->mutex);
		pthread_mutex_unlock(&a->mutex);
	}
	else
	{
		pthread_mutex_unlock(&a->mutex);
		pthread_mutex_unlock(&b->mutex);
	}
}

/**
 * @brief Try to acquire both dongles needed to compile.
 *
 * This function blocks until both dongles are free and their cooldown
 * has passed. It logs "has taken a dongle" twice when successful.
 *
 * Note: FIFO/EDF fairness is mandatory and will be added later in
 * scheduler.c. 
 * @param coder Pointer to the coder.
 * @return 0 on success, 1 if simulation stopped before acquiring.
 * 
	while simulation running:
		lock both dongles
		if both free AND cooldown passed:
			take them
			log
			unlock
			return success
		unlock
		wait a little

 */
int	dongles_acquire_pair(t_coder *coder)
{
	long		now;
	t_dongle	*l;
	t_dongle	*r;

	l = coder->left;
	r = coder->right;
	while (is_running(coder->cfg))
	{
		lock_pair(l, r);
		now = now_ms();
		if (!l->in_use && !r->in_use
			&& now >= l->available_after && now >= r->available_after)
		{
			l->in_use = 1;
			r->in_use = 1;
			log_state(coder->cfg, coder->id, "has taken a dongle");
			if (l != r)
				log_state(coder->cfg, coder->id, "has taken a dongle");
			unlock_pair(l, r);
			return (0);
		}
		unlock_pair(l, r);
		usleep(500);
	}
	return (1);
}

/**
 * @brief Release both dongles after compiling and apply cooldown.
 *
 * This function marks dongles as free, sets available_after to
 * now + dongle_cooldown, and wakes waiting coders. Cooldown is
 * mandatory
 *
 * @param coder Pointer to the coder.
 */
void	dongles_release_pair(t_coder *coder)
{
	long		now;
	t_dongle	*l;
	t_dongle	*r;

	l = coder->left;
	r = coder->right;
	lock_pair(l, r);
	now = now_ms();
	l->in_use = 0;
	l->available_after = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&l->cond);
	if (l != r)
	{
		r->in_use = 0;
		r->available_after = now + coder->cfg->dongle_cooldown;
		pthread_cond_broadcast(&r->cond);
	}
	unlock_pair(l, r);
}
