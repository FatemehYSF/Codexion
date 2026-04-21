/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 14:43:36 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 15:04:30 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Sleep for a given time, but stop early if simulation ends.
 *
 * This function sleeps in small steps to allow fast exit when the
 * monitor stops the simulation.
 *
 * @param cfg Pointer to the global configuration.
 * @param ms Time to sleep in milliseconds.
 */
static void	coder_sleep_ms(t_config *cfg, int ms)
{
	long	start;

	start = now_ms();
	while (is_running(cfg) && (now_ms() - start) < ms)
		usleep(500);
}

/**
 * @brief Execute the debugging phase for one coder.
 *
 * This logs the "is debugging" state and sleeps for time_to_debug.
 *
 * @param coder Pointer to the coder structure.
 */
static void	do_debug(t_coder *coder)
{
	log_state(coder->cfg, coder->id, "is debugging");
	coder_sleep_ms(coder->cfg, coder->cfg->time_to_debug);
}

/**
 * @brief Execute the refactoring phase for one coder.
 *
 * This logs the "is refactoring" state and sleeps for time_to_refactor.
 *
 * @param coder Pointer to the coder structure.
 */
static void	do_refactor(t_coder *coder)
{
	log_state(coder->cfg, coder->id, "is refactoring");
	coder_sleep_ms(coder->cfg, coder->cfg->time_to_refactor);
}

/**
 * @brief Execute the compiling phase for one coder.
 *
 * This updates last_compile_start at compile start (burnout reset),
 * logs "is compiling", sleeps for time_to_compile, and increments
 * compile_count when done.
 *
 * @param coder Pointer to the coder structure.
 */
static void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->cfg->state_mutex);
	coder->last_compile_start = now_ms();
	pthread_mutex_unlock(&coder->cfg->state_mutex);
	log_state(coder->cfg, coder->id, "is compiling");
	coder_sleep_ms(coder->cfg, coder->cfg->time_to_compile);
	pthread_mutex_lock(&coder->cfg->state_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->cfg->state_mutex);
}

/**
 * @brief Main routine for a coder thread.
 *
 * The coder repeatedly acquires two dongles, compiles, releases them,
 * then debugs and refactors until the simulation stops.
 *
 * @param arg Pointer to the coder structure (t_coder *).
 * @return NULL when the thread exits.
 */
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (is_running(coder->cfg))
	{
		if (dongles_acquire_pair(coder))
			break ;
		if (!is_running(coder->cfg))
		{
			dongles_release_pair(coder);
			break ;
		}
		do_compile(coder);
		dongles_release_pair(coder);
		if (!is_running(coder->cfg))
			break ;
		do_debug(coder);
		if (!is_running(coder->cfg))
			break ;
		do_refactor(coder);
	}
	return (NULL);
}
