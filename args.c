/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 16:07:41 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/20 16:17:51 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
/**
 * @brief Check if a string contains only digits.
 *
 * This function verifies that the given string is not empty
 * and that every character is between '0' and '9'.
 *
 * @param str The string to check.
 * @return 1 if the string is a valid number, 0 otherwise.
 */
static int	is_number(char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

/**
 * @brief Convert a numeric string to a positive integer.
 *
 * This function checks if the string is a valid number,
 * converts it to an integer, and detects overflow.
 *
 * @param str The string to convert.
 * @return The converted integer, or -1 on error.
 */
static int	parse_positive_int(char *str)
{
	long	value;
	int		i;

	if (!is_number(str))
		return (-1);
	value = 0;
	i = 0;
	while (str[i])
	{
		value = value * 10 + (str[i] - '0');
		if (value > 2147483647)
			return (-1);
		i++;
	}
	return ((int)value);
}

/**
 * @brief Parse and validate all numeric arguments.
 *
 * This function reads numeric values from the argument list,
 * stores them in the config structure, and checks for invalid
 * or negative values.
 *
 * @param cfg Pointer to the configuration structure.
 * @param av Argument vector.
 * @return 0 if values are valid, 1 on error.
 */
static int	parse_numbers(t_config *cfg, char **av)
{
	cfg->number_of_coders = parse_positive_int(av[1]);
	cfg->time_to_burnout = parse_positive_int(av[2]);
	cfg->time_to_compile = parse_positive_int(av[3]);
	cfg->time_to_debug = parse_positive_int(av[4]);
	cfg->time_to_refactor = parse_positive_int(av[5]);
	cfg->number_of_compiles_required = parse_positive_int(av[6]);
	cfg->dongle_cooldown = parse_positive_int(av[7]);
	if (cfg->number_of_coders <= 0
		|| cfg->time_to_burnout < 0
		|| cfg->time_to_compile < 0
		|| cfg->time_to_debug < 0
		|| cfg->time_to_refactor < 0
		|| cfg->number_of_compiles_required < 0
		|| cfg->dongle_cooldown < 0)
		return (1);
	return (0);
}

/**
 * @brief Parse the scheduler argument.
 *
 * This function checks if the scheduler argument is "fifo"
 * or "edf" and stores the corresponding value in config.
 *
 * @param cfg Pointer to the configuration structure.
 * @param arg Scheduler argument string.
 * @return 0 on success, 1 on error.
 */
static int	parse_scheduler(t_config *cfg, char *arg)
{
	if (strcmp(arg, "fifo") == 0)
		cfg->scheduler = 0;
	else if (strcmp(arg, "edf") == 0)
		cfg->scheduler = 1;
	else
		return (1);
	return (0);
}

/**
 * @brief Parse and validate all program arguments.
 *
 * This function checks the argument count, parses numeric
 * arguments, validates the scheduler, and initializes
 * the simulation state.
 *
 * @param cfg Pointer to the configuration structure.
 * @param ac Argument count.
 * @param av Argument vector.
 * @return 0 if arguments are valid, 1 otherwise.
 */
int	parse_args(t_config *cfg, int ac, char **av)
{
	if (ac != 9)
		return (1);
	if (parse_numbers(cfg, av))
		return (1);
	if (parse_scheduler(cfg, av[8]))
		return (1);
	cfg->simulation_running = 1;
	pthread_mutex_init(&cfg->state_mutex, NULL);
	pthread_mutex_init(&cfg->log_mutex, NULL);
	return (0);
}
