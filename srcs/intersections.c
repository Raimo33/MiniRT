/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:15:20 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 16:59:44 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

double	intersect_ray_sphere(const t_ray ray, const t_shape *shape)
{
	const t_vector		oc = vec_sub(ray.origin, shape->sphere.center);
	const double		b = 2.0 * vec_dot(oc, ray.direction);
	const double		c = vec_dot(oc, oc) - shape->sphere.squared_radius;
	const double		discriminant = b * b - 4 * c;
	double				t[2];

	if (discriminant < 0)
		return (-1);
	else
	{
		t[0] = (-b - sqrt(discriminant)) / 2;
		t[1] = (-b + sqrt(discriminant)) / 2;
		if (t[0] < 0 && t[1] < 0)
			return (-1);
		if (t[0] > 0 && t[1] > 0)
			return (fmin(t[0], t[1]));
		else if (t[0] > 0)
			return (t[0]);
		else if (t[1] > 0)
			return (t[1]);
		return (0);
	}
}

double	intersect_ray_plane(const t_ray ray, const t_shape *shape)
{
	const t_plane	plane = shape->plane;
	double			denom;
	double			t;

	denom = vec_dot(plane.normal, ray.direction);
	if (fabs(denom) > EPSILON)
	{
		t = vec_dot(vec_sub(plane.center, ray.origin), plane.normal) / denom;
		if (t >= 0)
			return (t);
	}
	return (-1);
}

double	intersect_ray_triangle(const t_ray ray, const t_shape *s)
{
	t_vector		edges12[2];
	t_vector		h;
	double			a;
	double			uvtf[4];
	t_vector		sq[2];

	edges12[0] = vec_sub(s->triangle.vertices[1], s->triangle.vertices[0]);
	edges12[1] = vec_sub(s->triangle.vertices[2], s->triangle.vertices[0]);
	h = vec_cross(ray.direction, edges12[1]);
	a = vec_dot(edges12[0], h);
	if (a > -EPSILON && a < EPSILON)
		return (-1);
	uvtf[3] = 1.0f / a;
	sq[0] = vec_sub(ray.origin, s->triangle.vertices[0]);
	uvtf[0] = uvtf[3] * vec_dot(sq[0], h);
	if (uvtf[0] < 0.0f || uvtf[0] > 1.0f)
		return (-1);
	sq[1] = vec_cross(sq[0], edges12[0]);
	uvtf[1] = uvtf[3] * vec_dot(ray.direction, sq[1]);
	if (uvtf[1] < 0.0f || uvtf[0] + uvtf[1] > 1.0f)
		return (-1);
	uvtf[2] = uvtf[3] * vec_dot(edges12[1], sq[1]);
	if (uvtf[2] > EPSILON)
		return (uvtf[2]);
	return (-1);
}

double	intersect_ray_cone(const t_ray ray, const t_shape *s)
{
	const t_vector	center = s->cone.base_center;
	const t_vector	vod[3] = {s->cone.direction, ray.origin, ray.direction};
	const t_vector	co = vec_sub(vod[1], center);
	double			abc[3];
	double			t[3];

	abc[0] = vec_dot(vod[2], vod[0]) * vec_dot(vod[2], vod[0])
		- s->cone.costheta_squared;
	abc[1] = 2 * (vec_dot(vod[2], vod[0]) * vec_dot(co, vod[0])
			- vec_dot(vod[2], co) * s->cone.costheta_squared);
	abc[2] = vec_dot(co, vod[0]) * vec_dot(co, vod[0])
		- vec_dot(co, co) * s->cone.costheta_squared;
	t[2] = abc[1] * abc[1] - 4 * abc[0] * abc[2];
	if (t[2] < 0)
		return (-1.0);
	t[0] = (-abc[1] - sqrt(t[2])) / (2 * abc[0]);
	t[1] = (-abc[1] + sqrt(t[2])) / (2 * abc[0]);
	if (t[0] > 0 && t[1] > 0)
		return (fmin(t[0], t[1]));
	else if (t[0] > 0)
		return (t[0]);
	else if (t[1] > 0)
		return (t[1]);
	return (-1.0f);
}

inline bool	ray_intersects_aabb(const t_ray r,
	const t_point bou_mx, const t_point bou_min)
{
	const double	r_d_or[6] = {r.direction.x, r.direction.y,
		r.direction.z, r.origin.x, r.origin.y, r.origin.z};
	const double	bb_min_max[6] = {bou_min.x, bou_min.y,
		bou_min.z, bou_mx.x, bou_mx.y, bou_mx.z};
	double			t_min_max[2];
	uint8_t			i;
	double			t[2];

	t_min_max[0] = FLT_MIN;
	t_min_max[1] = FLT_MAX;
	i = -1;
	while (++i < 3)
	{
		t[0] = (bb_min_max[i] - r_d_or[i + 3]) * 1.0f / r_d_or[i];
		t[1] = (bb_min_max[i + 3] - r_d_or[i + 3]) * 1.0f / r_d_or[i];
		if (1.0f / r_d_or[i] < 0.0f)
			ft_swap(&t[0], &t[1]);
		t_min_max[0] = fmax(t[0], t_min_max[0]);
		t_min_max[1] = fmin(t[1], t_min_max[1]);
		if (t_min_max[1] <= t_min_max[0])
			return (false);
	}
	return (true);
}
