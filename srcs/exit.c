/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 21:30:12 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 13:56:08 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static void	octree_clear(t_octree *node, const uint16_t depth);
static void	free_shape(void *shape);

void	ft_quit(const uint8_t id, char *msg)
{
	if (!msg)
		perror("miniRT");
	else
	{
		ft_putstr_fd("miniRT: ", 2);
		ft_putstr_fd(msg, 2);
		ft_putstr_fd("\n", 2);
	}
	destroy_scene(get_scene(NULL));
	exit(id);
}

int	close_win(t_hook_data *hook_data)
{
	uint16_t	i;
	t_mlx_data	*win_data;

	win_data = hook_data->win_data;
	destroy_scene(hook_data->scene);
	i = 0;
	while (i < win_data->n_images)
		mlx_destroy_image(win_data->mlx, win_data->images[i++]);
	free(win_data->images);
	free(win_data->addrresses);
	free(win_data->viewport_x);
	free(win_data->viewport_y);
	mlx_destroy_window(win_data->mlx, win_data->win);
	mlx_destroy_display(win_data->mlx);
	free(win_data->mlx);
	free(hook_data);
	exit(0);
	return (0);
}

static void	octree_clear(t_octree *node, const uint16_t depth)
{
	uint8_t	i;

	if (!node)
		return ;
	if (node->children)
	{
		i = 0;
		while (i < 8)
			octree_clear(node->children[i++], depth + 1);
		free(node->children);
	}
	if (depth != 0)
		ft_lstclear(&node->shapes, NULL);
	free(node);
}

void	destroy_scene(t_scene *scene)
{
	if (!scene)
		return ;
	if (scene->octree && scene->octree->children)
		octree_clear(scene->octree, 0);
	else
		free(scene->octree);
	ft_lstclear(&scene->shapes, &free_shape);
	ft_lstclear(&scene->lights, &free);
	ft_lstclear(&scene->cameras, &free);
	free(scene->amblight);
}

static void	free_shape(void *shape)
{
	t_shape	*s;

	s = (t_shape *)shape;
	if (s->material->texture)
	{
		free(s->material->texture->path);
		s->material->texture->path = NULL;
		free(s->material->texture);
		s->material->texture = NULL;
	}
	free(s->material);
	s->material = NULL;
	free(s);
}
