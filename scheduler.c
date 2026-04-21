/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 17:00:00 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 17:10:00 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Restore heap property from index i downwards.
 *
 * @param d Pointer to the dongle heap container.
 * @param cfg Global configuration.
 * @param i Start index.
 */
static void	heapify_down(t_dongle *d, t_config *cfg, int i)
{
	int		l;
	int		r;
	int		best;
	t_req	tmp;

	while (1)
	{
		l = i * 2 + 1;
		r = i * 2 + 2;
		best = i;
		if (l < d->heap_size && req_better(cfg, &d->heap[l], &d->heap[best]))
			best = l;
		if (r < d->heap_size && req_better(cfg, &d->heap[r], &d->heap[best]))
			best = r;
		if (best == i)
			break ;
		tmp = d->heap[i];
		d->heap[i] = d->heap[best];
		d->heap[best] = tmp;
		i = best;
	}
}

/**
 * @brief Initialize scheduler heap for one dongle.
 *
 * @param d Pointer to the dongle.
 * @param cap Initial capacity.
 * @return 0 on success, 1 on error.
 */
int	scheduler_init(t_dongle *d, int cap)
{
	d->heap = (t_req *)malloc(sizeof(t_req) * cap);
	if (!d->heap)
		return (1);
	d->heap_size = 0;
	d->heap_cap = cap;
	d->seq = 0;
	return (0);
}

/**
 * @brief Free scheduler heap memory for one dongle.
 *
 * @param d Pointer to the dongle.
 */
void	scheduler_clear(t_dongle *d)
{
	if (d->heap)
		free(d->heap);
	d->heap = NULL;
	d->heap_size = 0;
	d->heap_cap = 0;
	d->seq = 0;
}

/**
 * @brief Push a coder request into the dongle scheduler heap.
 *
 * @param d Pointer to the dongle.
 * @param c Pointer to the coder.
 * @return 0 on success, 1 on error.
 */
int	scheduler_push(t_dongle *d, t_coder *c)
{
	int		i;
	int		p;
	t_req	tmp;

	if (d->heap_size >= d->heap_cap)
		return (1);
	i = d->heap_size++;
	d->heap[i].coder_id = c->id;
	d->heap[i].arrival_seq = ++d->seq;
	d->heap[i].deadline_ms = c->last_compile_start + c->cfg->time_to_burnout;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (req_better(c->cfg, &d->heap[p], &d->heap[i]))
			break ;
		tmp = d->heap[p];
		d->heap[p] = d->heap[i];
		d->heap[i] = tmp;
		i = p;
	}
	return (0);
}

/**
 * @brief Pop the next coder id from the dongle scheduler heap.
 *
 * @param d Pointer to the dongle.
 * @param cfg Global configuration.
 * @param coder_id Output: selected coder id.
 * @return 0 on success, 1 if empty.
 */
int	scheduler_pop(t_dongle *d, t_config *cfg, int *coder_id)
{
	if (d->heap_size <= 0)
		return (1);
	*coder_id = d->heap[0].coder_id;
	d->heap[0] = d->heap[--d->heap_size];
	heapify_down(d, cfg, 0);
	return (0);
}
