/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup_scene.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:25:57 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 15:37:57 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

int	boxes_overlap(const t_point box1_top, const t_point box1_bottom,
	const t_point box2_top, const t_point box2_bottom)
{
	return (box1_bottom.x <= box2_top.x && box1_top.x >= box2_bottom.x
		&& box1_bottom.y <= box2_top.y && box1_top.y >= box2_bottom.y
		&& box1_bottom.z <= box2_top.z && box1_top.z >= box2_bottom.z);
}

void	setup_scene(t_scene *scene)
{
	set_shapes_data(scene);
	set_world_extremes(scene);
	fill_octree(scene->octree, scene->shapes, OCTREE_DEPTH,
		(t_boxes){scene->world_max, scene->world_min});
}

void	set_cylinder_data(t_shape *shape)
{
	t_cylinder	*cylinder;
	t_vector	center_to_cap;

	cylinder = &shape->cylinder;
	cylinder->squared_radius = cylinder->radius * cylinder->radius;
	cylinder->direction = vec_normalize(cylinder->direction);
	cylinder->half_height = cylinder->height / 2;
	center_to_cap = vec_scale(cylinder->half_height,
			cylinder->direction);
	cylinder->top_cap_center = vec_add(cylinder->center, center_to_cap);
	cylinder->bottom_cap_center = vec_sub(cylinder->center,
			center_to_cap);
}

void	set_cone_data(t_shape *shape)
{
	double	cos_theta;

	cos_theta = cos(atan2(shape->cone.radius, shape->cone.height));
	shape->cone.costheta_squared = cos_theta * cos_theta;
	shape->cone.direction = vec_normalize(shape->cone.direction);
}

void	set_shapes_data(t_scene *scene)
{
	t_shape		*s;
	t_list		*node;

	node = scene->shapes;
	while (node)
	{
		s = (t_shape *)node->content;
		set_bounding_box(s);
		if (s->e_type == TRIANGLE)
			s->triangle.normal
				= vec_normalize(vec_cross(vec_sub(s->triangle.vertices[1],
							s->triangle.vertices[0]),
						vec_sub(s->triangle.vertices[2],
							s->triangle.vertices[0])));
		else if (s->e_type == SPHERE)
			s->sphere.squared_radius = s->sphere.radius * s->sphere.radius;
		else if (s->e_type == PLANE)
			s->plane.normal = vec_normalize(s->plane.normal);
		else if (s->e_type == CYLINDER)
			set_cylinder_data(s);
		else if (s->e_type == CONE)
			set_cone_data(s);
		node = node->next;
	}
}
