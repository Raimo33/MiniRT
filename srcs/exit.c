/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:30:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/04 12:20:46 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void destroy_scene(t_scene scene);
// static void octree_clear(t_octree *node);

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
	mlx_destroy_image(hook_data->win_data->mlx, hook_data->win_data->main_img);
	mlx_destroy_display(hook_data->win_data->mlx);
	free(hook_data->win_data->mlx);
	free(hook_data);
	exit(0);
	return (0);
}

static void destroy_scene(t_scene scene)
{
	ft_lstclear(&scene.lights, NULL);
	//octree_clear(scene.octree);
	free(scene.octree);
}

// static void octree_clear(t_octree *node)
// {
	

	
// 	free(node);
// }