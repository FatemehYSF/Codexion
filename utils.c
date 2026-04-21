/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:04:04 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 15:11:55 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Check if the simulation is still running.
 *
 * This function safely reads the shared simulation_running
 * flag using the state mutex.
 *
 * @param cfg Pointer to the global configuration.
 * @return 1 if the simulation is running, 0 otherwise.
 */
int	is_running(t_config *cfg)
{
	int	running;

	pthread_mutex_lock(&cfg->state_mutex);
	running = cfg->simulation_running;
	pthread_mutex_unlock(&cfg->state_mutex);
	return (running);
}
