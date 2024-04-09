/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:30:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/09 23:48:48 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void destroy_scene(t_scene *scene);
static void octree_clear(t_octree *node);
static void	free_shape(void *shape);

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
	mlx_destroy_image(hook_data->win_data->mlx, hook_data->win_data->img);
	mlx_destroy_window(hook_data->win_data->mlx, hook_data->win_data->win);
	mlx_destroy_display(hook_data->win_data->mlx);
	free(hook_data->win_data->mlx);
	free(hook_data);
	exit(0);
	return (0);
}

static void	octree_clear(t_octree *node)
{
	if (!node)
		return ;
	if (node->children)
	{
		for (uint8_t i = 0; i < 8; i++)
		{
			octree_clear(node->children[i]);
			node->children[i] = NULL;
		}
		free(node->children);
	}
	free(node);
}

static void destroy_scene(t_scene *scene)
{
	ft_lstclear(&scene->shapes, &free_shape);
	ft_lstclear(&scene->lights, &free);
	octree_clear(scene->octree);
	free(scene->random_bias_vectors);
	free(scene->camera);
}

static void	free_shape(void *shape)
{
	t_shape *s;
	
	s = (t_shape *)shape;
	free(s->material);
	free(s);
}