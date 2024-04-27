/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene3.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 15:13:06 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

t_list	*get_shapes_inside_box(t_list *shapes, t_point box_top,
	t_point box_bottom)
{
	t_list	*inside_shapes;
	t_shape	*current_shape;

	inside_shapes = NULL;
	while (shapes)
	{
		current_shape = (t_shape *)shapes->content;
		if (boxes_overlap(box_top, box_bottom,
				current_shape->bb_max, current_shape->bb_min))
			ft_lstadd_front(&inside_shapes, ft_lstnew(current_shape));
		shapes = shapes->next;
	}
	return (inside_shapes);
}

void	set_world_extremes(t_scene *scene)
{
	t_list		*shapes;
	t_shape		*shape;
	t_point		world_max;
	t_point		world_min;

	world_max = (t_point){0, 0, 0};
	world_min = (t_point){0, 0, 0};
	shapes = scene->shapes;
	if (!shapes)
		return ;
	shape = (t_shape *)shapes->content;
	while (shapes)
	{
		shape = shapes->content;
		world_max = (t_point){fmax(world_max.x, shape->bb_max.x),
			fmax(world_max.y, shape->bb_max.y),
			fmax(world_max.z, shape->bb_max.z)};
		world_min = (t_point){fmin(world_min.x, shape->bb_min.x),
			fmin(world_min.y, shape->bb_min.y),
			fmin(world_min.z, shape->bb_min.z)};
		shapes = shapes->next;
	}
	scene->world_min = world_min;
	scene->world_max = world_max;
}

void	set_bounding_box(t_shape *shape)
{
	static void (*const	get_bb_funcs[])(t_shape *)
		= {&set_bb_sphere, &set_bb_cylinder,
		&set_bb_triangle, &set_bb_cone, &set_bb_plane};
	const uint8_t		n_shapes
		= sizeof(get_bb_funcs) / sizeof(get_bb_funcs[0]);
	uint8_t				i;

	i = 0;
	while (i < n_shapes)
	{
		if (shape->e_type == i)
		{
			get_bb_funcs[i](shape);
			return ;
		}
		i++;
	}
}

void	set_bb_sphere(t_shape *shape)
{
	shape->bb_min = (t_point){shape->sphere.center.x - shape->sphere.radius,
		shape->sphere.center.y - shape->sphere.radius,
		shape->sphere.center.z - shape->sphere.radius};
	shape->bb_max = (t_point){shape->sphere.center.x + shape->sphere.radius,
		shape->sphere.center.y + shape->sphere.radius,
		shape->sphere.center.z + shape->sphere.radius};
}
