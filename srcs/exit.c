/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:30:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/11 18:52:36 by craimond         ###   ########.fr       */
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
	uint16_t	i;
	t_mlx_data	*win_data;

	win_data = hook_data->win_data;
	destroy_scene(hook_data->scene);
	i = 0;
	while (i < win_data->n_images)
		mlx_destroy_image(win_data->mlx, win_data->images[i++]);
	mlx_destroy_window(win_data->mlx, win_data->win);
	mlx_destroy_display(win_data->mlx);
	free(win_data->mlx);
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
	ft_lstclear(&scene->cameras, &free);
	octree_clear(scene->octree);
}

static void	free_shape(void *shape)
{
	t_shape *s;
	
	s = (t_shape *)shape;
	free(s->material);
	free(s);
}