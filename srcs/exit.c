/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:30:12 by craimond          #+#    #+#             */
/*   Updated: 2024/03/23 21:32:45 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

static void destroy_scene(t_scene scene);

void ft_quit(uint8_t id, char *msg)
{
	if (errno != 0)
		perror("miniRT");
	else
		printf("miniRT: %s\n", msg);
	exit(id);
}

int close_win(t_hook_data *hook_data)
{
	destroy_scene(hook_data->scene);
	mlx_destroy_window(hook_data->win_data->mlx, hook_data->win_data->win);
	mlx_destroy_image(hook_data->win_data->mlx, hook_data->win_data->img);
	mlx_destroy_display(hook_data->win_data->mlx);
	free(hook_data->win_data->mlx);
	free(hook_data);
	exit(0);
	return (0);
}

static void destroy_scene(t_scene scene)
{
	ft_lstclear(&scene.lights, NULL);
	ft_lstclear(&scene.spheres, NULL);
	ft_lstclear(&scene.planes, NULL);
	ft_lstclear(&scene.cylinders, NULL);
}