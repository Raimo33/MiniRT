/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:15:20 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 15:54:36 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static double	intersect_cylinder_side(const t_ray ray,
					const t_cylinder *cylinder, const double dot_ray_cylinder);
static double	intersect_cylinder_caps(const t_ray ray,
					const t_cylinder *cylinder, const double dot_ray_cylinder);
static double	intersect_cylinder_cap(const t_ray ray,
					const t_cylinder *cylinder, const t_vector extreme_center,
					const double dot_ray_cylinder);

double	intersect_ray_cylinder(const t_ray ray, const t_shape *shape)
{
	const double	dot_ray_cylinder
		= vec_dot(ray.direction, shape->cylinder.direction);
	const double	t1
		= intersect_cylinder_side(ray, &shape->cylinder, dot_ray_cylinder);
	const double	t2
		= intersect_cylinder_caps(ray, &shape->cylinder, dot_ray_cylinder);

	if (t2 > 0 && (t1 < 0 || t2 < t1))
		return (t2);
	return (t1);
}

static double	intersect_cylinder_caps(const t_ray ray,
	const t_cylinder *cylinder, const double dot_ray_cylinder)
{
	const double	t1
		= intersect_cylinder_cap(ray, cylinder,
			cylinder->top_cap_center, dot_ray_cylinder);
	const double	t2
		= intersect_cylinder_cap(ray, cylinder,
			cylinder->bottom_cap_center, dot_ray_cylinder);

	if (t1 > 0 && (t1 < t2 || t2 < 0))
		return (t1);
	return (t2);
}

static double	intersect_cylinder_cap(const t_ray ray,
	const t_cylinder *cylinder, const t_vector extreme_center,
	const double dot_ray_cylinder)
{
	double			t;
	double			t_cap;
	t_vector		p_cap;
	t_vector		res;

	t = -1;
	if (fabs(dot_ray_cylinder) > EPSILON)
	{
		t_cap = vec_dot(vec_sub(extreme_center, ray.origin),
				cylinder->direction) / dot_ray_cylinder;
		p_cap = vec_add(ray.origin, vec_scale(t_cap, ray.direction));
		res = vec_sub(p_cap, extreme_center);
		if (vec_dot(res, res) <= cylinder->squared_radius && t_cap > 0)
			t = t_cap;
	}
	return (t);
}

static double	while_func(const double t, const t_cylinder *cylinder,
	const t_ray ray)
{
	t_point			point;
	t_vector		vec_from_center_to_point;
	double			projection_lenght;

	if (t > 0)
	{
		point = vec_add(ray.origin, vec_scale(t, ray.direction));
		vec_from_center_to_point = vec_sub(point, cylinder->center);
		projection_lenght
			= vec_dot(vec_from_center_to_point, cylinder->direction);
		if (fabs(projection_lenght) <= cylinder->half_height)
			return (t);
	}
	return (-1);
}

static double	intersect_cylinder_side(const t_ray ray,
	const t_cylinder *cylinder, const double drc)
{
	const t_vector	oc = vec_sub(ray.origin, cylinder->center);
	const double	dot_oc_cylinder = vec_dot(oc, cylinder->direction);
	double			abc[3];
	double			t[4];

	t[2] = abc[1] * abc[1] - 4 * abc[0] * abc[2];
	abc[0] = 1 - pow(drc, 2);
	abc[1] = 2 * (vec_dot(ray.direction, oc) - (drc * dot_oc_cylinder));
	abc[2] = vec_dot(oc, oc) - pow(dot_oc_cylinder, 2)
		- cylinder->squared_radius;
	if (t[2] < 0)
		return (-1);
	t[0] = (-abc[1] - sqrt(t[2])) / (abc[0] * 2);
	t[1] = (-abc[1] + sqrt(t[2])) / (abc[0] * 2);
	t[3] = while_func(t[0], cylinder, ray);
	if (t[3] > 0)
		return (t[3]);
	return (while_func(t[1], cylinder, ray));
}
