/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 12:49:20 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/20 12:49:50 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>

/*
** Forward declarations
*/

typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;
typedef struct s_config	t_config;
typedef struct s_req	t_req;

struct s_req
{
	int		coder_id;
	long	arrival_seq;
	long	deadline_ms;
};

/*
** Config structure
*/

struct s_config
{
	int					number_of_coders;
	int					time_to_burnout;
	int					time_to_compile;
	int					time_to_debug;
	int					time_to_refactor;
	int					number_of_compiles_required;
	int					dongle_cooldown;
	int					scheduler;
	long				start_time_ms;
	int					simulation_running;
	pthread_mutex_t		state_mutex;
	pthread_mutex_t		log_mutex;
};

/*
** Coder structure
*/

struct s_coder
{
	int			id;
	pthread_t	thread;
	t_dongle	*left;
	t_dongle	*right;
	long		last_compile_start;
	int			compile_count;
	t_config	*cfg;
};

/*
** Dongle structure
*/

struct s_dongle
{
	int				id;
	int				in_use;
	long			available_after;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;

	t_req			*heap;
	int				heap_size;
	int				heap_cap;
	long			seq;

};

/*
** Functions
*/

int		parse_args(t_config *cfg, int ac, char **av);
int		init_all(t_config *cfg, t_coder **coders, t_dongle **dongles);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
long	now_ms(void);
void	log_state(t_config *cfg, int id, char *msg);
int		is_running(t_config *cfg);
int		dongles_acquire_pair(t_coder *coder);
void	dongles_release_pair(t_coder *coder);
int		req_better(t_config *cfg, t_req *a, t_req *b);

#endif