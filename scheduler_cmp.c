/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_cmp.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fatemeh <fatemeh@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 16:19:20 by fatemeh           #+#    #+#             */
/*   Updated: 2026/04/21 17:31:16 by fatemeh          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	req_better(t_config *cfg, t_req *a, t_req *b)
{
	if (cfg->scheduler == 0)
		return (a->arrival_seq < b->arrival_seq);
	if (a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->arrival_seq != b->arrival_seq)
		return (a->arrival_seq < b->arrival_seq);
	return (a->coder_id < b->coder_id);
}
