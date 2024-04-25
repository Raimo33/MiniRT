/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 21:15:20 by craimond          #+#    #+#             */
/*   Updated: 2024/04/25 14:12:18 by craimond         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headers/minirt.h"

static double	intersect_cylinder_side(const t_ray ray, const t_cylinder *cylinder, const double dot_ray_cylinder);
static double	intersect_cylinder_caps(const t_ray ray, const t_cylinder *cylinder, const double dot_ray_cylinder);
static double	intersect_cylinder_cap(const t_ray ray, const t_cylinder *cylinder, const t_vector extreme_center, const double dot_ray_cylinder);

double	intersect_ray_sphere(const t_ray ray, const t_shape *shape)
{
	const t_sphere		sphere = shape->sphere;
	const t_vector		oc = vec_sub(ray.origin, sphere.center);
	const double		b = 2.0 * vec_dot(oc, ray.direction);
	const double		c = vec_dot(oc, oc) - sphere.squared_radius;
	const double		discriminant = b * b - 4 * c;
	double				sqrt_discriminant;
	double				t[2];

	if (discriminant < 0)
		return (-1);
	else
	{
		sqrt_discriminant = sqrt(discriminant);
		t[0] = (-b - sqrt_discriminant) / 2;
		t[1] = (-b + sqrt_discriminant) / 2;
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

static double	intersect_cylinder_caps(const t_ray ray, const t_cylinder *cylinder, const double dot_ray_cylinder)
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

static double	intersect_cylinder_cap(const t_ray ray, const t_cylinder *cylinder, const t_vector extreme_center, const double dot_ray_cylinder)
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

static double	intersect_cylinder_side(const t_ray ray,
	const t_cylinder *cylinder, const double dot_ray_cylinder)
{
	const t_vector	oc = vec_sub(ray.origin, cylinder->center);
	const double	dot_oc_cylinder = vec_dot(oc, cylinder->direction);
	const double	a = 1 - pow(dot_ray_cylinder, 2);
	const double	b = 2 * (vec_dot(ray.direction, oc) - (dot_ray_cylinder * dot_oc_cylinder));
	const double	c = vec_dot(oc, oc) - pow(dot_oc_cylinder, 2) - cylinder->squared_radius;
	const double	discriminant = b * b - 4 * a * c;
	double			t[2];
	double			sqrt_discriminant;
	double			valid_t;
	uint8_t			i;
	double			two_times_a;
	t_point			point;
	t_vector		vec_from_center_to_point;
	double			projection_lenght;

	if (discriminant < 0)
		return (-1);
	sqrt_discriminant = sqrt(discriminant);
	two_times_a = 2 * a;
	t[0] = (-b - sqrt_discriminant) / two_times_a;
	t[1] = (-b + sqrt_discriminant) / two_times_a;
	valid_t = -1;
	i = 0;
	while (i < 2)
	{
		if (t[i] > 0)
		{
			point = vec_add(ray.origin, vec_scale(t[i], ray.direction));
			vec_from_center_to_point = vec_sub(point, cylinder->center);
			projection_lenght
				= vec_dot(vec_from_center_to_point, cylinder->direction);
			if (fabs(projection_lenght) <= cylinder->half_height)
			{
				valid_t = t[i];
				break ;
			}
		}
		i++;
	}
	return (valid_t);
}

double	intersect_ray_triangle(const t_ray ray, const t_shape *shape)
{
	const t_vector	edge1 = vec_sub(shape->triangle.vertices[1],
			shape->triangle.vertices[0]);
	const t_vector	edge2 = vec_sub(shape->triangle.vertices[2],
			shape->triangle.vertices[0]);
	const t_vector	h = vec_cross(ray.direction, edge2);
	const double	a = vec_dot(edge1, h);
	double			u;
	double			v;
	double			t;
	double			f;
	t_vector		s;
	t_vector		q;

	if (a > -EPSILON && a < EPSILON)
		return (-1);
	f = 1.0f / a;
	s = vec_sub(ray.origin, shape->triangle.vertices[0]);
	u = f * vec_dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return (-1);
	q = vec_cross(s, edge1);
	v = f * vec_dot(ray.direction, q);
	if (v < 0.0f || u + v > 1.0f)
		return (-1);
	t = f * vec_dot(edge2, q);
	if (t > EPSILON)
		return (t);
	return (-1);
}

double intersect_ray_cone(const t_ray ray, const t_shape *shape)
{
	const t_cone	cone = shape->cone;
	const t_vector	center = cone.base_center;
	const t_vector	v = cone.direction;
	const t_vector	o = ray.origin;
	const t_vector	d = ray.direction;
	const t_vector	co = vec_sub(o, center);
	const double	a = vec_dot(d, v) * vec_dot(d, v) - cone.costheta_squared;
	const double	b = 2.0 * (vec_dot(d, v) * vec_dot(co, v) - vec_dot(d, co) * cone.costheta_squared);
	const double	c = vec_dot(co, v) * vec_dot(co, v) - vec_dot(co, co) * cone.costheta_squared;
	const double	discriminant = b * b - 4 * a * c;
	double			t[2];
	double			sqrt_discriminant;
	double			two_times_a;

	if (discriminant < 0)
		return (-1.0);
	sqrt_discriminant = sqrt(discriminant);
	two_times_a = 2 * a;
	t[0] = (-b - sqrt_discriminant) / two_times_a;
	t[1] = (-b + sqrt_discriminant) / two_times_a;
	if (t[0] > 0 && t[1] > 0)
		return (fmin(t[0], t[1]));
	else if (t[0] > 0)
		return (t[0]);
	else if (t[1] > 0)
		return (t[1]);
	else
		return (-1.0f);
}

inline bool	ray_intersects_aabb(const t_ray ray, const t_point bounding_box_max, const t_point bounding_box_min)
{
	const double	ray_direction[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
	const double	bb_max[3] = {bounding_box_max.x, bounding_box_max.y, bounding_box_max.z};
	const double	bb_min[3] = {bounding_box_min.x, bounding_box_min.y, bounding_box_min.z};
	const double	ray_origin[3] = {ray.origin.x, ray.origin.y, ray.origin.z};
	double			tmin;
	double			tmax;
	uint8_t			i;
	double			invd;
	double			t[2];
	double			temp;

	tmin = -FLT_MAX;
	tmax = FLT_MAX;
	i = 0;
	while (i < 3)
	{
		invd = 1.0f / ray_direction[i];
		t[0] = (bb_min[i] - ray_origin[i]) * invd;
		t[1] = (bb_max[i] - ray_origin[i]) * invd;
		if (invd < 0.0f)
		{
			temp = t[0];
			t[0] = t[1];
			t[1] = temp;
		}
		tmin = fmax(t[0], tmin);
		tmax = fmin(t[1], tmax);
		if (tmax <= tmin)
			return (false);
		i++;
	}
	return (true);
}

inline t_point	ray_point_at_parameter(const t_ray ray, const double t)
{
	return ((t_point)
		{
			.x = ray.origin.x + t * ray.direction.x,
			.y = ray.origin.y + t * ray.direction.y,
			.z = ray.origin.z + t * ray.direction.z
		});
}
