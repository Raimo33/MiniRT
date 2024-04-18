/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intersections.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 13:16:18 by craimond          #+#    #+#             */
/*   Updated: 2024/04/18 13:45:56 by craimond         ###   ########.fr       */
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
    const double		a = 1;
    const double		b = 2.0 * vec_dot(oc, ray.direction);
    const double		c = vec_dot(oc, oc) - sphere.radius * sphere.radius;
    const double		discriminant = b * b - 4 * a * c;
	const double		two_times_a = 2 * a;

    if (discriminant < 0)
        return (-1);
    else
	{
		const double sqrt_discriminant = sqrt(discriminant);
        const double t1 = (-b - sqrt_discriminant) / two_times_a;
        const double t2 = (-b + sqrt_discriminant) / two_times_a;
        
        if (t1 < 0 && t2 < 0)
            return (-1);
        if (t1 > 0 && t2 > 0)
            return (t1 < t2 ? t1 : t2);
        else if (t1 > 0)
            return (t1);
        else if (t2 > 0)
            return (t2);
        return (0);
    }
}

double	intersect_ray_plane(const t_ray ray, const t_shape *shape)
{
	const t_plane	plane = shape->plane;
	double	denom;
	double	t;

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
	const double	dot_ray_cylinder = vec_dot(ray.direction, shape->cylinder.direction);
	const double	t1 = intersect_cylinder_side(ray, &shape->cylinder, dot_ray_cylinder);
	const double	t2 = intersect_cylinder_caps(ray, &shape->cylinder, dot_ray_cylinder);

	if (t2 > 0 && (t1 < 0 || t2 < t1))
		return (t2);
	return (t1);
}

static double	intersect_cylinder_caps(const t_ray ray, const t_cylinder *cylinder, const double dot_ray_cylinder)
{
    const double	t1 = intersect_cylinder_cap(ray, cylinder, cylinder->top_cap_center, dot_ray_cylinder);
    const double	t2 = intersect_cylinder_cap(ray, cylinder, cylinder->bottom_cap_center, dot_ray_cylinder);
	if (t1 > 0 && (t1 < t2 || t2 < 0))
		return (t1);
	return (t2);
}

static double	intersect_cylinder_cap(const t_ray ray, const t_cylinder *cylinder, const t_vector extreme_center, const double dot_ray_cylinder)
{
    const double	denom = dot_ray_cylinder;
	double 			t = -1;

    if (fabs(denom) > EPSILON)
	{
        const double 	t_cap = vec_dot(vec_sub(extreme_center, ray.origin), cylinder->direction) / denom;
        const t_vector	p_cap = vec_add(ray.origin, vec_scale(t_cap, ray.direction));
		const t_vector	res = vec_sub(p_cap, extreme_center);
        if (vec_dot(res, res) <= cylinder->squared_radius && t_cap > 0)
            t = t_cap;
    }
	return (t);
}

static double	intersect_cylinder_side(const t_ray ray, const t_cylinder *cylinder, const double dot_ray_cylinder)
{
	const t_vector	oc = vec_sub(ray.origin, cylinder->center);
	const double	dot_oc_cylinder = vec_dot(oc, cylinder->direction);
    const double	A = 1 - pow(dot_ray_cylinder, 2);
    const double	B = 2 * (vec_dot(ray.direction, oc) - (dot_ray_cylinder * dot_oc_cylinder));
    const double 	C = vec_dot(oc, oc) - pow(dot_oc_cylinder, 2) - cylinder->squared_radius;
	const double	discriminant = B * B - 4 * A * C;
	if (discriminant < 0)
		return (-1);
	const double	sqrt_discriminant = sqrt(discriminant);
	const double	two_times_a = 2 * A;
	const double	t[2] = {
		(-B - sqrt_discriminant) / two_times_a,
		(-B + sqrt_discriminant) / two_times_a
	};
	double	valid_t = -1;
	uint8_t	i = 0;
	while (i < 2)
	{
		if (t[i] > 0)
		{
			const t_point	point = vec_add(ray.origin, vec_scale(t[i], ray.direction));
			const t_vector	vec_from_center_to_point = vec_sub(point, cylinder->center);
			const double	projection_lenght = vec_dot(vec_from_center_to_point, cylinder->direction);

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
	const t_vector	edge1 = vec_sub(shape->triangle.vertices[1], shape->triangle.vertices[0]);
	const t_vector	edge2 = vec_sub(shape->triangle.vertices[2], shape->triangle.vertices[0]);
	const t_vector 	h = vec_cross(ray.direction, edge2);
	const double 	a = vec_dot(edge1, h);

	if (a > -EPSILON && a < EPSILON)
		return (-1);
	const double 	f = 1.0f / a;
	const t_vector	s = vec_sub(ray.origin, shape->triangle.vertices[0]);
	const double 	u = f * vec_dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return (-1);
    const t_vector	q = vec_cross(s, edge1);
    const double	v = f * vec_dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f)
		return (-1);
    const double	t = f * vec_dot(edge2, q);
    if (t > EPSILON)
        return (t);
    return (-1);
}

double intersect_ray_cone(const t_ray ray, const t_shape *shape)
{
    t_vector	CO = vec_sub(ray.origin, shape->cone.intersection_point);
    double		cos_alpha = cos(atan(shape->cone.radius / shape->cone.height));
    double		cos_alpha_squared = cos_alpha * cos_alpha;
    t_vector	V = vec_normalize(shape->cone.direction);
    double		V_dot_d = vec_dot(V, ray.direction);
    double		V_dot_CO = vec_dot(V, CO);
    double 		a = V_dot_d * V_dot_d - cos_alpha_squared;
    double		b = 2.0 * (V_dot_d * V_dot_CO - vec_dot(ray.direction, CO) * cos_alpha_squared);
    double		c = V_dot_CO * V_dot_CO - vec_dot(CO, CO) * cos_alpha_squared;
    double		discriminant = b * b - 4 * a * c;
    
	if (discriminant < 0)
		return (-1.0);

	double	sqrt_discriminant = sqrt(discriminant);
	double	two_times_a = 2 * a;
    double	t1 = (-b - sqrt_discriminant) / two_times_a;
    double	t2 = (-b + sqrt_discriminant) / two_times_a;
    
    if (t1 > 0 && t2 > 0)
        return (fmin(t1, t2));
    else if (t1 > 0)
        return (t1);
    else if (t2 > 0)
        return (t2);
    return (-1.0);
}

inline bool	ray_intersects_aabb(t_ray ray, t_point bounding_box_max, t_point bounding_box_min)
{
    double tmin = -FLT_MAX;
    double tmax = FLT_MAX;
	double ray_direction[3] = {ray.direction.x, ray.direction.y, ray.direction.z};
	double bb_max[3] = {bounding_box_max.x, bounding_box_max.y, bounding_box_max.z};
	double bb_min[3] = {bounding_box_min.x, bounding_box_min.y, bounding_box_min.z};
	double ray_origin[3] = {ray.origin.x, ray.origin.y, ray.origin.z};

    for (int i = 0; i < 3; i++)
	{
        double invD = 1.0f / ray_direction[i];
        double t0 = (bb_min[i] - ray_origin[i]) * invD;
        double t1 = (bb_max[i] - ray_origin[i]) * invD;
        if (invD < 0.0f)
		{
            double temp = t0;
            t0 = t1;
            t1 = temp;
        }
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax <= tmin)
            return (false);
    }
    return (true);
}

inline t_point ray_point_at_parameter(const t_ray ray, double t)
{
    return ((t_point)
	{
		.x = ray.origin.x + t * ray.direction.x,
		.y = ray.origin.y + t * ray.direction.y,
		.z = ray.origin.z + t * ray.direction.z
	});
}