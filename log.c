/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 16:00:00 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 16:00:00 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Get the current simulation timestamp in milliseconds.
 *
 * This returns the elapsed time since cfg->start_time_ms.
 *
 * @param cfg Pointer to the global configuration.
 * @return Elapsed milliseconds since simulation start.
 */
static long	timestamp_ms(t_config *cfg)
{
	return (now_ms() - cfg->start_time_ms);
}

/**
 * @brief Check if logging is allowed (simulation still running).
 *
 * This protects access to cfg->simulation_running with state_mutex.
 *
 * @param cfg Pointer to the global configuration.
 * @return 1 if logging should happen, 0 otherwise.
 */
static int	can_log(t_config *cfg)
{
	int	running;

	pthread_mutex_lock(&cfg->state_mutex);
	running = cfg->simulation_running;
	pthread_mutex_unlock(&cfg->state_mutex);
	return (running);
}

/**
 * @brief Print a coder state line in a thread-safe way.
 *
 * Format: "<timestamp_ms> <id> <msg>"
 * Logging is serialized using cfg->log_mutex so lines never interleave.
 *
 * @param cfg Pointer to the global configuration.
 * @param id Coder id.
 * @param msg Message string (e.g. "is compiling").
 */
void	log_state(t_config *cfg, int id, char *msg)
{
	pthread_mutex_lock(&cfg->log_mutex);
	if (can_log(cfg))
		printf("%ld %d %s\n", timestamp_ms(cfg), id, msg);
	pthread_mutex_unlock(&cfg->log_mutex);
}
