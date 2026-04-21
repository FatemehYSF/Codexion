/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 21:04:53 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 11:41:41 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Destroy all dongle mutexes and condition variables.
 *
 * This function releases synchronization resources created
 * for each dongle. It is used during cleanup on error.
 * Each dongle owns a mutex and a condition variable.
 * If initialization fails, these synchronization objects must be 
 * destroyed before exiting to avoid resource leaks and undefined behavior.
 *
 * @param dongles Pointer to the dongles array.
 * @param count Number of dongles.
 */
static void	destroy_dongles(t_dongle *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&dongles[i].cond);
		pthread_mutex_destroy(&dongles[i].mutex);
		i++;
	}
}

/**
 * @brief Free all allocated resources.
 *
 * This function frees memory allocated for coders and
 * dongles and destroys dongle synchronization primitives
 * to avoid memory leaks on initialization failure.
 *
 * @param coders Address of the coders array pointer.
 * @param dongles Address of the dongles array pointer.
 * @param count Number of dongles.
 */
static void	free_all(t_coder **coders, t_dongle **dongles, int count)
{
	if (dongles && *dongles)
	{
		destroy_dongles(*dongles, count);
		free(*dongles);
		*dongles = NULL;
	}
	if (coders && *coders)
	{
		free(*coders);
		*coders = NULL;
	}
}

/**
 * @brief Initialize all dongles.
 *
 * This function sets the initial state of each dongle,
 * initializes its mutex and condition variable, and marks
 * it as free and available at the start of the simulation.
 *
 * @param cfg Pointer to the global configuration.
 * @param dongles Pointer to the dongles array.
 * @return 0 on success, 1 on error.
 */
static int	init_dongles(t_config *cfg, t_dongle *dongles)
{
	int	i;

	i = 0;
	while (i < cfg->number_of_coders)
	{
		dongles[i].id = i + 1;
		dongles[i].in_use = 0;
		dongles[i].available_after = cfg->start_time_ms;
		if (pthread_mutex_init(&dongles[i].mutex, NULL) != 0)
			return (1);
		if (pthread_cond_init(&dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&dongles[i].mutex);
			return (1);
		}
		i++;
	}
	return (0);
}

/**
 * @brief Initialize all coders.
 *
 * This function assigns an ID to each coder, initializes
 * its counters and timing data, links it to the global
 * configuration, and assigns its left and right dongles
 * according to the circular topology.
 *
 * @param cfg Pointer to the global configuration.
 * @param coders Pointer to the coders array.
 * @param dongles Pointer to the dongles array.
 * @return 0 on success, 1 on error.
 */
static int	init_coders(t_config *cfg, t_coder *coders, t_dongle *dongles)
{
	int	i;
	int	n;

	n = cfg->number_of_coders;
	i = 0;
	while (i < n)
	{
		coders[i].id = i + 1;
		coders[i].left = &dongles[i];
		coders[i].right = &dongles[(i + 1) % n];
		coders[i].last_compile_start = cfg->start_time_ms;
		coders[i].compile_count = 0;
		coders[i].cfg = cfg;
		i++;
	}
	return (0);
}

/**
 * @brief Initialize all simulation structures.
 *
 * This function allocates memory for coders and dongles,
 * initializes them, builds the circular topology, and sets
 * the simulation start time before any thread is created.
 *
 * @param cfg Pointer to the global configuration.
 * @param coders Address of the coders array pointer.
 * @param dongles Address of the dongles array pointer.
 * @return 0 on success, 1 on error.
 */
int	init_all(t_config *cfg, t_coder **coders, t_dongle **dongles)
{
	int	n;

	n = cfg->number_of_coders;
	cfg->start_time_ms = now_ms();
	*coders = (t_coder *)malloc(sizeof(t_coder) * n);
	*dongles = (t_dongle *)malloc(sizeof(t_dongle) * n);
	if (!*coders || !*dongles)
	{
		free_all(coders, dongles, 0);
		return (1);
	}
	if (init_dongles(cfg, *dongles))
	{
		free_all(coders, dongles, 0);
		return (1);
	}
	if (init_coders(cfg, *coders, *dongles))
	{
		free_all(coders, dongles, n);
		return (1);
	}
	return (0);
}
