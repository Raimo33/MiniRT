/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   render2.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: egualand <egualand@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:24:04 by craimond          #+#    #+#             */
/*   Updated: 2024/04/27 16:11:08 by egualand         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

t_color	add_texture(const t_hit *hit_info)
{
	double					u;
	double					v;
	t_material				*material;
	int						iuv[2];

	material = hit_info->shape->material;
	if (material->is_checkerboard)
	{
		get_uv(hit_info, &u, &v);
		iuv[0] = floor(u * CHECKERBOARD_TILE_DENSITY);
		iuv[1] = floor(v * CHECKERBOARD_TILE_DENSITY);
		if ((iuv[0] + iuv[1]) % 2)
			return ((t_color){CC1 >> 16 & 255, CC1 >> 8 & 255, CC1 & 255});
		else
			return ((t_color){CC2 >> 16 & 255, CC2 >> 8 & 255, CC2 & 255});
	}
	else if (material->texture)
	{
		get_uv(hit_info, &u, &v);
		return (my_mlx_pixel_get(material->texture,
				u * material->texture->width, v * material->texture->height));
	}
	return (material->color);
}

t_hit	*trace_ray(const t_scene *scene, const t_ray ray)
{
	t_hit	*closest_hit;

	closest_hit = (t_hit *)calloc_p(1, sizeof(t_hit));
	*closest_hit = (t_hit)
	{
		.distance = FLT_MAX,
		.point = {0, 0, 0},
		.normal = {0, 0, 0},
		.shape = NULL,
	};
	traverse_octree(scene->octree, ray, closest_hit);
	if (closest_hit->distance == FLT_MAX)
		return (free(closest_hit), NULL);
	return (closest_hit);
}

void	traverse_octree(const t_octree *node, const t_ray ray,
	t_hit *closest_hit)
{
	uint8_t	i;

	if (!node || !ray_intersects_aabb(ray, node->box_top, node->box_bottom))
		return ;
	if (node->n_shapes == 1 || node->children == NULL)
		check_shapes_in_node(node, ray, closest_hit);
	else
	{
		i = 0;
		while (i < 8)
			traverse_octree(node->children[i++], ray, closest_hit);
	}
}

inline void	check_shapes_in_node(const t_octree *node,
	const t_ray ray, t_hit *closest_hit)
{
	t_list					*shapes;
	t_shape					*shape;
	double					t;
	static double (*const	intersect[])(const t_ray, const t_shape *)
		= {&intersect_ray_sphere, &intersect_ray_cylinder,
		&intersect_ray_triangle,
		&intersect_ray_cone, &intersect_ray_plane};

	shapes = node->shapes;
	while (shapes)
	{
		shape = (t_shape *)shapes->content;
		t = intersect[shape->e_type](ray, shape);
		if (t > 0 && t < closest_hit->distance)
			update_closest_hit(closest_hit, shape, t, ray);
		shapes = shapes->next;
	}
}

void	update_closest_hit(t_hit *closest_hit, t_shape *shape,
	const double t, const t_ray ray)
{
	closest_hit->distance = t;
	closest_hit->point = ray_point_at_parameter(ray, t);
	closest_hit->shape = shape;
	if (shape->e_type == TRIANGLE)
		closest_hit->normal = shape->triangle.normal;
	else if (shape->e_type == SPHERE)
		closest_hit->normal
			= vec_normalize(vec_sub(closest_hit->point, shape->sphere.center));
	else if (shape->e_type == CYLINDER)
		closest_hit->normal
			= get_cylinder_normal(shape->cylinder, closest_hit->point);
	else if (shape->e_type == PLANE)
		closest_hit->normal = shape->plane.normal;
	else if (shape->e_type == CONE)
		closest_hit->normal = get_cone_normal(shape->cone, closest_hit->point);
}
