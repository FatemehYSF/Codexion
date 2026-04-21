/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 16:03:40 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 17:21:43 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Create all coder threads.
 *
 * @param coders Pointer to the coders array.
 * @param cfg Pointer to the global configuration.
 * @return 0 on success, 1 on error.
 */
static int	start_coders(t_coder *coders, t_config *cfg)
{
	int	i;

	i = 0;
	while (i < cfg->number_of_coders)
	{
		if (pthread_create(&coders[i].thread, NULL,
				coder_routine, &coders[i]) != 0)
			return (1);
		i++;
	}
	return (0);
}

/**
 * @brief Join all coder threads.
 *
 * @param coders Pointer to the coders array.
 * @param cfg Pointer to the global configuration.
 */
static void	join_coders(t_coder *coders, t_config *cfg)
{
	int	i;

	i = 0;
	while (i < cfg->number_of_coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
}

/**
 * @brief Program entry point.
 *
 * Initializes the simulation, starts coder and monitor threads,
 * waits for termination, and frees all resources.
 *
 * @param ac Argument count.
 * @param av Argument vector.
 * @return 0 on success, 1 on error.
 */
int	main(int ac, char **av)
{
	t_config	cfg;
	t_coder		*coders;
	t_dongle	*dongles;
	pthread_t	monitor;

	coders = NULL;
	dongles = NULL;
	if (parse_args(&cfg, ac, av))
		return (1);
	if (init_all(&cfg, &coders, &dongles))
		return (1);
	if (start_coders(coders, &cfg))
		return (1);
	if (pthread_create(&monitor, NULL, monitor_routine, coders) != 0)
		return (1);
	pthread_join(monitor, NULL);
	join_coders(coders, &cfg);
	free(coders);
	free(dongles);
	return (0);
}
