/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:37:09 by fatemeh           #+#    #+#             */
/*   Updated: 2026/05/04 20:06:42 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Stop the simulation safely.
 *
 * This sets cfg->simulation_running to 0 using the state mutex.
 *
 * @param cfg Pointer to the global configuration.
 */
static void	stop_simulation(t_config *cfg)
{
	pthread_mutex_lock(&cfg->state_mutex);
	cfg->simulation_running = 0;
	pthread_mutex_unlock(&cfg->state_mutex);
}

/**
 * @brief Check if all coders finished required compiles.
 *
 * If number_of_compiles_required is 0, this returns 0 (not finished),
 * because it expects the simulation to stop only when a coder
 * burns out or when everyone reached the required count.
 *
 * @param coders Pointer to the coders array.
 * @return 1 if finished, 0 otherwise.
 */
static int	all_done(t_coder *coders)
{
	int			i;
	int			n;
	int			req;
	t_config	*cfg;

	cfg = coders[0].cfg;
	req = cfg->number_of_compiles_required;
	n = cfg->number_of_coders;
	pthread_mutex_lock(&cfg->state_mutex);
	i = 0;
	while (i < n)
	{
		if (coders[i].compile_count < req)
		{
			pthread_mutex_unlock(&cfg->state_mutex);
			return (0);
		}
		i++;
	}
	pthread_mutex_unlock(&cfg->state_mutex);
	return (1);
}

/**
 * @brief Check if any coder has burned out.
 *
 * Burnout happens if a coder did not start compiling within time_to_burnout
 * milliseconds since their last compile start (or simulation start).
 * 
 * @param coders Pointer to the coders array.
 * @return 1 if burnout detected, 0 otherwise.
 */
static int	check_burnout(t_coder *coders)
{
	int			i;
	int			n;
	long		now;
	long		last;
	t_config	*cfg;

	cfg = coders[0].cfg;
	n = cfg->number_of_coders;
	now = now_ms();
	pthread_mutex_lock(&cfg->state_mutex);
	i = 0;
	while (i < n)
	{
		last = coders[i].last_compile_start;
		if ((now - last) > cfg->time_to_burnout)
		{
			pthread_mutex_unlock(&cfg->state_mutex);
			log_state(cfg, coders[i].id, "burned out");
			stop_simulation(cfg);
			return (1);
		}
		i++;
	}
	pthread_mutex_unlock(&cfg->state_mutex);
	return (0);
}

/**
 * @brief Main routine of the monitor thread.
 *
 * This thread checks burnout and completion conditions and stops the
 * simulation when needed. Burnout log must appear within 10 ms.
 *
 * @param arg Pointer to coders array (t_coder *).
 * @return NULL when the thread exits.
 */
void	*monitor_routine(void *arg)
{
	t_coder	*coders;

	coders = (t_coder *)arg;
	while (is_running(coders[0].cfg))
	{
		if (check_burnout(coders))
			break ;
		if (all_done(coders))
		{
			stop_simulation(coders[0].cfg);
			break ;
		}
		usleep(500);
	}
	return (NULL);
}
