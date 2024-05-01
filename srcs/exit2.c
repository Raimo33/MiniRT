/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit2.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/01 15:18:41 by craimond          #+#    #+#             */
/*   Updated: 2024/05/01 15:19:42 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

int	close_win(t_hook_data *hook_data)
{
	t_mlx_data	*win_data;

	win_data = hook_data->win_data;
	destroy_scene(hook_data->scene);
	destroy_mlx_data(win_data);
	free(hook_data);
	exit(0);
	return (0);
}
